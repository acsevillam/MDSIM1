#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../../.." && pwd)"
source "${script_dir}/../../lib/md1_job_helpers.sh"

if [ "$#" -ne 9 ]; then
    echo "Usage: bash ${BASH_SOURCE[0]} <run_type> <cylinder_material> <cylinder_radius_mm> <cylinder_height_mm> <envelope_material> <envelope_thickness_mm> <output_dir> <timestamp> <job_dir>" >&2
    exit 1
fi

run_type="$1"
cylinder_material="$2"
cylinder_radius_mm="$3"
cylinder_height_mm="$4"
envelope_material="$5"
envelope_thickness_mm="$6"
output_dir="$7"
timestamp="$8"
job_dir="$9"

n_histories=200000000
n_replicas=10

project_root="$(md1_resolve_project_root_containing "${repo_root}" "input/detectors/cylinder/templates/CylinderCalibration.intmp")"
executable="$(md1_resolve_executable "${project_root}")"
template_macro="${project_root}/input/detectors/cylinder/templates/CylinderCalibration.intmp"
analysis_dir="$(dirname "${executable}")/analysis"
summary_file="${job_dir}/calculated_total_collected_charge_summary.csv"

md1_prepare_job_directory "${output_dir}" "${job_dir}"

echo "Timestamp for this execution: ${timestamp}"
echo "Executable: ${executable}"
echo "Template macro: ${template_macro}"
echo "Cylinder material: ${cylinder_material}"
echo "Cylinder radius: ${cylinder_radius_mm} mm"
echo "Cylinder height: ${cylinder_height_mm} mm"
echo "Envelope material: ${envelope_material}"
echo "Envelope thickness: ${envelope_thickness_mm} mm"
echo "Histories per replica: ${n_histories}"
echo "Number of replicas: ${n_replicas}"

printf "replica,calculated_total_collected_charge_nC,mc_err_nC,rms_nC\n" > "${summary_file}"

for ((replica=1; replica<=n_replicas; replica++)); do
    result_dir="${job_dir}/${replica}"
    mkdir -p "${result_dir}"

    macro_copy="${result_dir}/macro.in"
    cp "${template_macro}" "${macro_copy}"

    md1_cylinder_configure_calibration_macro \
        "${macro_copy}" \
        "${cylinder_radius_mm}" \
        "${cylinder_height_mm}" \
        "${cylinder_material}" \
        "0" \
        "${envelope_material}" \
        "${envelope_thickness_mm}"

    echo "Running replica ${replica}/${n_replicas}"
    time "${executable}" -m "${macro_copy}" -b on -v off -n "${n_histories}" > "${result_dir}/output.log"

    md1_move_analysis_contents "${analysis_dir}" "${result_dir}"

    charge_line=$(md1_extract_detector_results_line \
        "${result_dir}/output.log" \
        "Cylinder" \
        "cylinder[0]" \
        "(7)  Scaled collected charge ")

    if [ -z "${charge_line}" ]; then
        echo "Could not find cylinder results line (7) in ${result_dir}/output.log" >&2
        exit 1
    fi

    charge_pattern=': ([^[:space:]]+) nC mc_err = ([^[:space:]]+) nC .* rms = ([^[:space:]]+) nC$'
    if [[ "${charge_line}" =~ ${charge_pattern} ]]; then
        charge_nC="${BASH_REMATCH[1]}"
        mc_err_nC="${BASH_REMATCH[2]}"
        rms_nC="${BASH_REMATCH[3]}"
    else
        echo "Could not parse scaled collected charge line: ${charge_line}" >&2
        exit 1
    fi

    printf "%s,%s,%s,%s\n" "${replica}" "${charge_nC}" "${mc_err_nC}" "${rms_nC}" >> "${summary_file}"
done

CYLINDER_MATERIAL="${cylinder_material}" \
CYLINDER_RADIUS_MM="${cylinder_radius_mm}" \
CYLINDER_HEIGHT_MM="${cylinder_height_mm}" \
ENVELOPE_MATERIAL="${envelope_material}" \
ENVELOPE_THICKNESS_MM="${envelope_thickness_mm}" \
awk -F',' '
    NR==1 {next}
    {
        count += 1
        charge[count] = $2 + 0
        internal_mc_err[count] = $3 + 0
        internal_rms[count] = $4 + 0
        charge_sum += charge[count]
        internal_mc_err_sum += internal_mc_err[count]
        internal_rms_sum += internal_rms[count]
    }
    END {
        if (count == 0) {
            exit 1
        }
        charge_mean = charge_sum / count
        internal_mc_err_mean = internal_mc_err_sum / count
        internal_rms_mean = internal_rms_sum / count

        charge_var = 0
        internal_mc_err_var = 0
        internal_rms_var = 0
        if (count > 1) {
            for (i = 1; i <= count; i++) {
                charge_var += (charge[i] - charge_mean) * (charge[i] - charge_mean)
                internal_mc_err_var += (internal_mc_err[i] - internal_mc_err_mean) * (internal_mc_err[i] - internal_mc_err_mean)
                internal_rms_var += (internal_rms[i] - internal_rms_mean) * (internal_rms[i] - internal_rms_mean)
            }
            charge_stddev = sqrt(charge_var / (count - 1))
            internal_mc_err_stddev = sqrt(internal_mc_err_var / (count - 1))
            internal_rms_stddev = sqrt(internal_rms_var / (count - 1))
        } else {
            charge_stddev = 0
            internal_mc_err_stddev = 0
            internal_rms_stddev = 0
        }

        charge_err = charge_stddev / sqrt(count)
        internal_mc_err_of_mean = internal_mc_err_stddev / sqrt(count)
        internal_rms_err = internal_rms_stddev / sqrt(count)
        charge_err_rel = (charge_mean != 0) ? 100 * charge_err / ((charge_mean < 0) ? -charge_mean : charge_mean) : 0
        internal_mc_err_rel = (internal_mc_err_mean != 0) ? 100 * internal_mc_err_of_mean / ((internal_mc_err_mean < 0) ? -internal_mc_err_mean : internal_mc_err_mean) : 0
        internal_rms_rel = (internal_rms_mean != 0) ? 100 * internal_rms_err / ((internal_rms_mean < 0) ? -internal_rms_mean : internal_rms_mean) : 0

        printf "Collected charge over %d replicas:\n", count
        printf "Configuration:\n"
        printf "  cylinder material = %s\n", ENVIRON["CYLINDER_MATERIAL"]
        printf "  cylinder radius = %s mm\n", ENVIRON["CYLINDER_RADIUS_MM"]
        printf "  cylinder height = %s mm\n", ENVIRON["CYLINDER_HEIGHT_MM"]
        printf "  envelope material = %s\n", ENVIRON["ENVELOPE_MATERIAL"]
        printf "  envelope thickness = %s mm\n", ENVIRON["ENVELOPE_THICKNESS_MM"]
        printf "  mean = %.12g nC\n", charge_mean
        printf "  stddev = %.12g nC\n", charge_stddev
        printf "  err = %.12g nC (%.6g %%)\n", charge_err, charge_err_rel
        printf "Per-run internal mc_err over %d replicas:\n", count
        printf "  mean = %.12g nC\n", internal_mc_err_mean
        printf "  stddev = %.12g nC\n", internal_mc_err_stddev
        printf "  err = %.12g nC (%.6g %%)\n", internal_mc_err_of_mean, internal_mc_err_rel
        printf "Per-run internal rms over %d replicas:\n", count
        printf "  mean = %.12g nC\n", internal_rms_mean
        printf "  stddev = %.12g nC\n", internal_rms_stddev
        printf "  err = %.12g nC (%.6g %%)\n", internal_rms_err, internal_rms_rel
    }
' "${summary_file}" | tee "${job_dir}/calculated_total_collected_charge_average.txt"

echo "Summary written to ${summary_file}"
echo "Average report written to ${job_dir}/calculated_total_collected_charge_average.txt"
