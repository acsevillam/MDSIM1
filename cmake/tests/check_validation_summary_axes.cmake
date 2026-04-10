if(NOT DEFINED WORKING_DIRECTORY)
  message(FATAL_ERROR "WORKING_DIRECTORY must be defined")
endif()

set(test_dir "${WORKING_DIRECTORY}/Testing/validation_summary_axes")
file(REMOVE_RECURSE "${test_dir}")
file(MAKE_DIRECTORY "${test_dir}")

set(common_header "replica,iX,iY,iZ,dose_cGy,rms_cGy,n_entries,total_value_gy,total_val2_gy2\n")
set(validation_macro "${WORKING_DIRECTORY}/input/dosimetry/Validation.in")
set(run_validation_script "${WORKING_DIRECTORY}/jobs/dosimetry/run_validation.sh")

function(query_scorer_metadata scorer_name out_axis_label out_axis_mode out_n_bins)
  execute_process(
    COMMAND /bin/bash -lc "source \"${run_validation_script}\" && read_scorer_axis_metadata \"${validation_macro}\" \"${scorer_name}\" \"${WORKING_DIRECTORY}\""
    WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    RESULT_VARIABLE meta_result
    OUTPUT_VARIABLE meta_stdout
    ERROR_VARIABLE meta_stderr
    TIMEOUT 60
  )

  if(NOT meta_result EQUAL 0)
    message(FATAL_ERROR
      "Failed to read scorer metadata for ${scorer_name}.\n"
      "${meta_stdout}\n${meta_stderr}"
    )
  endif()

  string(STRIP "${meta_stdout}" meta_stdout)
  string(REPLACE "\t" ";" meta_fields "${meta_stdout}")
  list(LENGTH meta_fields meta_field_count)
  if(NOT meta_field_count EQUAL 5)
    message(FATAL_ERROR
      "Unexpected scorer metadata format for ${scorer_name}: ${meta_stdout}"
    )
  endif()

  list(GET meta_fields 0 axis_label)
  list(GET meta_fields 1 axis_mode)
  list(GET meta_fields 4 n_bins)

  set(${out_axis_label} "${axis_label}" PARENT_SCOPE)
  set(${out_axis_mode} "${axis_mode}" PARENT_SCOPE)
  set(${out_n_bins} "${n_bins}" PARENT_SCOPE)
endfunction()

function(write_fixture_csv scorer_name first_dose first_rms first_entries second_dose second_rms second_entries)
  query_scorer_metadata("${scorer_name}" axis_label axis_mode n_bins)

  file(WRITE "${test_dir}/${scorer_name}_by_replica.csv"
    "${common_header}"
    "1,0,0,0,${first_dose},${first_rms},${first_entries},0,0\n"
  )

  if(NOT axis_mode STREQUAL "iso")
    math(EXPR last_index "${n_bins} - 1")
    set(ix 0)
    set(iy 0)
    set(iz 0)

    if(axis_mode STREQUAL "x")
      set(ix "${last_index}")
    elseif(axis_mode STREQUAL "y")
      set(iy "${last_index}")
    elseif(axis_mode STREQUAL "z")
      set(iz "${last_index}")
    else()
      message(FATAL_ERROR "Unsupported axis mode '${axis_mode}' for ${scorer_name}")
    endif()

    file(APPEND "${test_dir}/${scorer_name}_by_replica.csv"
      "1,${ix},${iy},${iz},${second_dose},${second_rms},${second_entries},0,0\n"
    )
  endif()
endfunction()

write_fixture_csv("pdd" "1.0" "0.1" "10" "2.0" "0.2" "11")
write_fixture_csv("pddBuildup" "3.0" "0.3" "12" "4.0" "0.4" "13")
write_fixture_csv("latProfile_x" "5.0" "0.5" "14" "6.0" "0.6" "15")
write_fixture_csv("latProfile_y" "7.0" "0.7" "16" "8.0" "0.8" "17")
write_fixture_csv("atIsocenter" "9.0" "0.9" "18" "0.0" "0.0" "0")

set(shell_script [=[
source "@WORKING_DIRECTORY@/jobs/dosimetry/run_validation.sh"
macro_path="@WORKING_DIRECTORY@/input/dosimetry/Validation.in"
project_root="@WORKING_DIRECTORY@"
phsp_prefix="beam/test_validation_"

write_scorer_summary "pdd" \
  "@TEST_DIR@/pdd_by_replica.csv" \
  "@TEST_DIR@/pdd_summary.csv" \
  "@TEST_DIR@/pdd_summary.txt" \
  "${macro_path}" \
  "${project_root}" \
  "${phsp_prefix}"

write_scorer_summary "pddBuildup" \
  "@TEST_DIR@/pddBuildup_by_replica.csv" \
  "@TEST_DIR@/pddBuildup_summary.csv" \
  "@TEST_DIR@/pddBuildup_summary.txt" \
  "${macro_path}" \
  "${project_root}" \
  "${phsp_prefix}"

write_scorer_summary "latProfile_x" \
  "@TEST_DIR@/latProfile_x_by_replica.csv" \
  "@TEST_DIR@/latProfile_x_summary.csv" \
  "@TEST_DIR@/latProfile_x_summary.txt" \
  "${macro_path}" \
  "${project_root}" \
  "${phsp_prefix}"

write_scorer_summary "latProfile_y" \
  "@TEST_DIR@/latProfile_y_by_replica.csv" \
  "@TEST_DIR@/latProfile_y_summary.csv" \
  "@TEST_DIR@/latProfile_y_summary.txt" \
  "${macro_path}" \
  "${project_root}" \
  "${phsp_prefix}"

write_scorer_summary "atIsocenter" \
  "@TEST_DIR@/atIsocenter_by_replica.csv" \
  "@TEST_DIR@/atIsocenter_summary.csv" \
  "@TEST_DIR@/atIsocenter_summary.txt" \
  "${macro_path}" \
  "${project_root}" \
  "${phsp_prefix}"
]=])

set(TEST_DIR "${test_dir}")
string(CONFIGURE "${shell_script}" shell_script @ONLY)

execute_process(
  COMMAND /bin/bash -lc "${shell_script}"
  WORKING_DIRECTORY "${WORKING_DIRECTORY}"
  RESULT_VARIABLE summary_result
  OUTPUT_VARIABLE summary_stdout
  ERROR_VARIABLE summary_stderr
  TIMEOUT 60
)

if(NOT summary_result EQUAL 0)
  message(FATAL_ERROR
    "Validation summary axis test failed while invoking write_scorer_summary.\n"
    "${summary_stdout}\n${summary_stderr}"
  )
endif()

set(verify_script_path "${test_dir}/verify_validation_summary_axes.py")
file(WRITE "${verify_script_path}" [=[
from collections import OrderedDict
from pathlib import Path
import csv
import math
import sys

working_dir = Path(sys.argv[1]).resolve()
test_dir = Path(sys.argv[2]).resolve()
macro_file = working_dir / "input" / "dosimetry" / "Validation.in"


def strip_comments(line):
    return line.split("#", 1)[0].split("//", 1)[0].strip()


def unit_scale_to_mm(unit):
    return {"mm": 1.0, "cm": 10.0, "m": 1000.0}[unit]


def resolve_include(current_file, include_path_text):
    include_path = Path(include_path_text)
    candidates = []
    if include_path.is_absolute():
        candidates.append(include_path)
    else:
        candidates.append((current_file.parent / include_path).resolve())
        candidates.append((working_dir / include_path).resolve())

    for candidate in candidates:
        if candidate.exists():
            return candidate.resolve()
    raise FileNotFoundError(include_path_text)


def expand_macro(path, expanded_lines, visiting):
    path = path.resolve()
    if path in visiting:
        raise RuntimeError(f"Recursive /control/execute detected while reading {path}")

    visiting.add(path)
    for raw_line in path.read_text().splitlines():
        line = strip_comments(raw_line)
        if not line:
            continue
        parts = line.split()
        if parts[0] == "/control/execute":
            expand_macro(resolve_include(path, parts[1]), expanded_lines, visiting)
        else:
            expanded_lines.append(line)
    visiting.remove(path)


expanded_lines = []
expand_macro(macro_file, expanded_lines, set())

meshes = {}
current_mesh = None
for line in expanded_lines:
    parts = line.split()
    command = parts[0]

    if command == "/score/create/boxMesh":
        current_mesh = parts[1]
        meshes[current_mesh] = {}
        continue

    if current_mesh is None:
        continue

    if command == "/score/mesh/boxSize":
        scale = unit_scale_to_mm(parts[4])
        meshes[current_mesh]["box_size_mm"] = tuple(float(parts[i]) * scale for i in range(1, 4))
    elif command == "/score/mesh/translate/xyz":
        scale = unit_scale_to_mm(parts[4])
        meshes[current_mesh]["translate_mm"] = tuple(float(parts[i]) * scale for i in range(1, 4))
    elif command == "/score/mesh/nBin":
        meshes[current_mesh]["n_bins"] = tuple(int(parts[i]) for i in range(1, 4))
    elif command == "/score/close":
        current_mesh = None


def scorer_metadata(name):
    mesh = meshes[name]
    box_size_mm = mesh["box_size_mm"]
    translate_mm = mesh["translate_mm"]
    n_bins = mesh["n_bins"]

    axis_candidates = [
        ("x", n_bins[0], translate_mm[0] - box_size_mm[0], translate_mm[0] + box_size_mm[0]),
        ("y", n_bins[1], translate_mm[1] - box_size_mm[1], translate_mm[1] + box_size_mm[1]),
        ("z", n_bins[2], translate_mm[2] - box_size_mm[2], translate_mm[2] + box_size_mm[2]),
    ]
    variable_axes = [candidate for candidate in axis_candidates if candidate[1] > 1]

    if len(variable_axes) == 0:
        return {
            "axis_label": "position_mm",
            "axis_mode": "iso",
            "axis_min_mm": 0.0,
            "axis_max_mm": 0.0,
            "n_bins": 1,
        }

    axis_mode, axis_n_bins, axis_min_mm, axis_max_mm = variable_axes[0]
    axis_label = {
        "x": "x_mm",
        "y": "y_mm",
        "z": "depth_mm",
    }[axis_mode]
    return {
        "axis_label": axis_label,
        "axis_mode": axis_mode,
        "axis_min_mm": axis_min_mm,
        "axis_max_mm": axis_max_mm,
        "n_bins": axis_n_bins,
    }


def compute_axis_value(meta, ix, iy, iz):
    if meta["axis_mode"] == "iso":
        return 0.0

    if meta["axis_mode"] == "x":
        voxel_index = float(ix)
    elif meta["axis_mode"] == "y":
        voxel_index = float(iy)
    else:
        voxel_index = float(iz)

    step_mm = (meta["axis_max_mm"] - meta["axis_min_mm"]) / meta["n_bins"]
    center_mm = meta["axis_min_mm"] + ((voxel_index + 0.5) * step_mm)
    if meta["axis_mode"] == "z":
        return meta["axis_max_mm"] - center_mm
    return center_mm


def unique_fixture_rows(path):
    rows = OrderedDict()
    with path.open(newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            key = (row["iX"], row["iY"], row["iZ"])
            rows.setdefault(key, row)
    return list(rows.values())


def assert_close(actual, expected, description):
    if not math.isclose(actual, expected, rel_tol=0.0, abs_tol=1e-9):
        raise AssertionError(
            f"Unexpected value for {description}. Expected {expected}, got {actual}."
        )


for scorer_name in ["pdd", "pddBuildup", "latProfile_x", "latProfile_y", "atIsocenter"]:
    meta = scorer_metadata(scorer_name)
    fixture_rows = unique_fixture_rows(test_dir / f"{scorer_name}_by_replica.csv")
    with (test_dir / f"{scorer_name}_summary.csv").open(newline="") as handle:
        summary_rows = list(csv.DictReader(handle))

    if not summary_rows:
        raise AssertionError(f"Expected summary rows for {scorer_name}.")

    axis_label = meta["axis_label"]
    if axis_label not in summary_rows[0]:
        raise AssertionError(
            f"Expected axis label '{axis_label}' in {scorer_name}_summary.csv, got {list(summary_rows[0].keys())}."
        )

    if len(summary_rows) != len(fixture_rows):
        raise AssertionError(
            f"Expected {len(fixture_rows)} rows in {scorer_name}_summary.csv, got {len(summary_rows)}."
        )

    for fixture_row, summary_row in zip(fixture_rows, summary_rows):
        expected_axis = compute_axis_value(
            meta,
            fixture_row["iX"],
            fixture_row["iY"],
            fixture_row["iZ"],
        )
        actual_axis = float(summary_row[axis_label])
        assert_close(actual_axis, expected_axis, f"{scorer_name} axis value")

print("Validation summary axis reconstruction test passed")
]=])

execute_process(
  COMMAND python3 "${verify_script_path}" "${WORKING_DIRECTORY}" "${test_dir}"
  WORKING_DIRECTORY "${WORKING_DIRECTORY}"
  RESULT_VARIABLE verify_result
  OUTPUT_VARIABLE verify_stdout
  ERROR_VARIABLE verify_stderr
  TIMEOUT 60
)

if(NOT verify_result EQUAL 0)
  message(FATAL_ERROR
    "Validation summary axis verification failed.\n"
    "${verify_stdout}\n${verify_stderr}"
  )
endif()

message(STATUS "Validation summary axis reconstruction test passed")
