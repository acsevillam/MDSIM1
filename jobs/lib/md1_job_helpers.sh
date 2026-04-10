#!/bin/bash

md1_resolve_project_root_containing() {
    local base_repo_root="$1"
    local relative_path="$2"
    local candidate_root=""

    candidate_root="${base_repo_root}"
    if [ -f "${candidate_root}/${relative_path}" ]; then
        echo "${candidate_root}"
        return
    fi

    candidate_root="$(cd "${base_repo_root}/.." && pwd)/MDSIM1"
    if [ -f "${candidate_root}/${relative_path}" ]; then
        echo "${candidate_root}"
        return
    fi

    echo "Could not find ${relative_path} starting from ${base_repo_root}" >&2
    return 1
}

md1_resolve_executable() {
    local project_root="$1"

    if [ -x "${project_root}/MultiDetector1" ]; then
        echo "${project_root}/MultiDetector1"
        return
    fi

    if [ -x "${project_root}/build/MultiDetector1" ]; then
        echo "${project_root}/build/MultiDetector1"
        return
    fi

    echo "Could not find MultiDetector1 executable under ${project_root} or ${project_root}/build" >&2
    return 1
}

md1_prepare_job_directory() {
    local output_dir="$1"
    local job_dir="$2"

    mkdir -p "${output_dir}"
    mkdir -p "${job_dir}"
}

md1_replace_placeholder() {
    local file_path="$1"
    local placeholder="$2"
    local replacement="$3"
    local escaped_placeholder=""
    local escaped_replacement=""

    escaped_placeholder=$(printf '%s' "${placeholder}" | sed 's/[.[\*^$()+?{|\\]/\\&/g; s/&/\\&/g')
    escaped_replacement=$(printf '%s' "${replacement}" | sed 's/[&|]/\\&/g')

    sed "s|${escaped_placeholder}|${escaped_replacement}|g" "${file_path}" > "${file_path}.tmp"
    mv "${file_path}.tmp" "${file_path}"
}

md1_replace_line_matching() {
    local file_path="$1"
    local line_regex="$2"
    local replacement="$3"

    awk -v line_regex="${line_regex}" -v replacement="${replacement}" '
        $0 ~ line_regex { print replacement; next }
        { print }
    ' "${file_path}" > "${file_path}.tmp"
    mv "${file_path}.tmp" "${file_path}"
}

md1_insert_line_after_first_match() {
    local file_path="$1"
    local line_regex="$2"
    local inserted_line="$3"

    awk -v line_regex="${line_regex}" -v inserted_line="${inserted_line}" '
        {
            print
            if (!inserted && $0 ~ line_regex) {
                print inserted_line
                inserted = 1
            }
        }
    ' "${file_path}" > "${file_path}.tmp"
    mv "${file_path}.tmp" "${file_path}"
}

md1_clear_analysis_outputs() {
    local analysis_dir="$1"
    shift

    local output_name=""
    for output_name in "$@"; do
        rm -f "${analysis_dir}/${output_name}"
    done
}

md1_collect_required_analysis_outputs() {
    local analysis_dir="$1"
    local destination_dir="$2"
    shift 2

    local output_name=""
    for output_name in "$@"; do
        if [ ! -f "${analysis_dir}/${output_name}" ]; then
            echo "Expected analysis output was not produced: ${analysis_dir}/${output_name}" >&2
            return 1
        fi
        mv "${analysis_dir}/${output_name}" "${destination_dir}/${output_name}"
    done
}

md1_move_analysis_contents() {
    local analysis_dir="$1"
    local destination_dir="$2"

    if [ -d "${analysis_dir}" ]; then
        mv "${analysis_dir}"/* "${destination_dir}/" 2>/dev/null || true
    fi
}

md1_parse_single_voxel_dose_out() {
    local scorer_file="$1"
    local n_histories="$2"

    awk -F',' -v n_histories="${n_histories}" '
        /^[[:space:]]*#/ { next }
        NF < 6 { next }
        {
            total_value_gy = $4 + 0
            total_val2_gy2 = $5 + 0
            n_entries = $6 + 0

            dose_gy = total_value_gy / n_histories
            dose_cgy = dose_gy / 1e-2

            rms_cgy = 0
            if (n_histories > 1) {
                variance_gy2 = (n_histories / (n_histories - 1.0)) * ((total_val2_gy2 / n_histories) - (dose_gy * dose_gy))
                if (variance_gy2 < 0 && variance_gy2 > -1e-18) {
                    variance_gy2 = 0
                }
                if (variance_gy2 < 0) {
                    printf "Negative variance while parsing %s: %.12g\n", FILENAME, variance_gy2 > "/dev/stderr"
                    exit 1
                }
                rms_cgy = sqrt(variance_gy2) / 1e-2
            }

            printf "%.12g,%.12g,%s\n", dose_cgy, rms_cgy, n_entries
            found = 1
            exit
        }
        END {
            if (!found) {
                exit 1
            }
        }
    ' "${scorer_file}"
}

md1_cube_configure_calibration_macro() {
    local macro_copy="$1"
    local cube_side_mm="$2"
    local cube_material="$3"
    local cube_z_cm="$4"
    local envelope_material="$5"
    local envelope_thickness_mm="$6"
    local eof_policy="${7:-}"

    md1_replace_placeholder "${macro_copy}" "**CubeSide**" "${cube_side_mm}"
    md1_replace_placeholder "${macro_copy}" "**CubeMaterial**" "${cube_material}"
    md1_replace_placeholder "${macro_copy}" "**PhspPrefix**" "beam/Varian_TrueBeam6MV_"
    md1_replace_placeholder "${macro_copy}" "**Jaw1X**" "5"
    md1_replace_placeholder "${macro_copy}" "**Jaw2X**" "5"
    md1_replace_placeholder "${macro_copy}" "**Jaw1Y**" "5"
    md1_replace_placeholder "${macro_copy}" "**Jaw2Y**" "5"
    md1_replace_placeholder "${macro_copy}" "**GantryAngle**" "0"
    md1_replace_placeholder "${macro_copy}" "**CollimatorAngle**" "0"
    md1_replace_placeholder "${macro_copy}" "**MU**" "1"
    md1_replace_placeholder "${macro_copy}" "**WaterBoxZ**" "-10"
    md1_replace_placeholder "${macro_copy}" "**CubeZ**" "${cube_z_cm}"
    md1_replace_placeholder "${macro_copy}" "**CubeAngle**" "0"

    md1_replace_line_matching \
        "${macro_copy}" \
        "^/MultiDetector1/detectors/cube/setEnvelopeThickness .*" \
        "/MultiDetector1/detectors/cube/setEnvelopeThickness ${envelope_thickness_mm} mm"
    md1_replace_line_matching \
        "${macro_copy}" \
        "^/MultiDetector1/detectors/cube/setEnvelopeMaterial .*" \
        "/MultiDetector1/detectors/cube/setEnvelopeMaterial ${envelope_material}"

    if [ -n "${eof_policy}" ]; then
        md1_insert_line_after_first_match \
            "${macro_copy}" \
            "/MultiDetector1/beamline/clinac/phsp/setPrefix " \
            "/MultiDetector1/beamline/clinac/phsp/setEOFPolicy ${eof_policy}"
    fi
}
