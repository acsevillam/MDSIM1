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
n_replicas=5
cylinder_z_points=(
    11.0 10.9 10.8 10.7 10.6 10.5 10.4 10.3 10.2 10.1 10.0
    9.9 9.8 9.7 9.6 9.5 9.4 9.3 9.2 9.1 9.0
    8.9 8.8 8.7 8.6 8.5
)
sweep_description="Depth sweep: z from ${cylinder_z_points[0]} cm down to ${cylinder_z_points[$((${#cylinder_z_points[@]} - 1))]} cm in 0.1 cm steps."

project_root="$(md1_resolve_project_root_containing "${repo_root}" "input/detectors/cylinder/templates/CylinderCalibration.intmp")"
executable="$(md1_resolve_executable "${project_root}")"
template_macro="${project_root}/input/detectors/cylinder/templates/CylinderCalibration.intmp"
analysis_dir="$(dirname "${executable}")/analysis"
replica_csv="${job_dir}/pdd_by_replica.csv"
summary_csv="${job_dir}/pdd_summary.csv"
summary_txt="${job_dir}/pdd_summary.txt"

md1_prepare_job_directory "${output_dir}" "${job_dir}"

echo "Timestamp for this execution: ${timestamp}"
echo "Executable: ${executable}"
echo "Template macro: ${template_macro}"
echo "Cylinder material: ${cylinder_material}"
echo "Cylinder radius: ${cylinder_radius_mm} mm"
echo "Cylinder height: ${cylinder_height_mm} mm"
echo "Envelope material: ${envelope_material}"
echo "Envelope thickness: ${envelope_thickness_mm} mm"
echo "Histories per point: ${n_histories}"
echo "Number of replicas: ${n_replicas}"
echo "EOF policy: synthetic"
echo "${sweep_description}"

printf "replica,cylinder_z_cm,estimated_total_absorbed_dose_cGy,mc_err_cGy,mu_err_cGy,det_err_cGy,total_err_cGy,rms_cGy\n" > "${replica_csv}"

for ((replica=1; replica<=n_replicas; replica++)); do
    replica_dir="${job_dir}/${replica}"
    mkdir -p "${replica_dir}"

    echo "Running replica ${replica}/${n_replicas}"
    for cylinder_z_cm in "${cylinder_z_points[@]}"; do
        point_tag=$(printf "%05.2f" "${cylinder_z_cm}" | tr '.' 'p')
        result_dir="${replica_dir}/z_${point_tag}cm"
        mkdir -p "${result_dir}"

        macro_copy="${result_dir}/macro.in"
        cp "${template_macro}" "${macro_copy}"

        md1_cylinder_configure_calibration_macro \
            "${macro_copy}" \
            "${cylinder_radius_mm}" \
            "${cylinder_height_mm}" \
            "${cylinder_material}" \
            "${cylinder_z_cm}" \
            "${envelope_material}" \
            "${envelope_thickness_mm}" \
            "synthetic"

        time "${executable}" -m "${macro_copy}" -b on -v off -n "${n_histories}" > "${result_dir}/output.log"

        md1_move_analysis_contents "${analysis_dir}" "${result_dir}"

        estimated_line=$(md1_extract_detector_results_line \
            "${result_dir}/output.log" \
            "Cylinder" \
            "cylinder[0]" \
            "(8)  Estimated absorbed dose in water ")

        if [ -z "${estimated_line}" ]; then
            echo "Could not find cylinder results line (8) in ${result_dir}/output.log" >&2
            exit 1
        fi

        estimated_cGy=$(echo "${estimated_line}" | sed -E 's/^.*: ([^ ]+) cGy mc_err = .*$/\1/')
        mc_err_cGy=$(echo "${estimated_line}" | sed -E 's/^.* mc_err = ([^ ]+) cGy .*$/\1/')
        mu_err_cGy=$(echo "${estimated_line}" | sed -E 's/^.* mu_err = ([^ ]+) cGy .*$/\1/')
        det_err_cGy=$(echo "${estimated_line}" | sed -E 's/^.* det_err = ([^ ]+) cGy .*$/\1/')
        total_err_cGy=$(echo "${estimated_line}" | sed -E 's/^.* total_err = ([^ ]+) cGy .*$/\1/')
        rms_cGy=$(echo "${estimated_line}" | sed -E 's/^.* rms = ([^ ]+) cGy$/\1/')

        if [ -z "${estimated_cGy}" ] || [ -z "${mc_err_cGy}" ] || [ -z "${mu_err_cGy}" ] || [ -z "${det_err_cGy}" ] || [ -z "${total_err_cGy}" ] || [ -z "${rms_cGy}" ]; then
            echo "Could not parse cylinder estimated absorbed dose line: ${estimated_line}" >&2
            exit 1
        fi

        printf "%s,%s,%s,%s,%s,%s,%s,%s\n" \
            "${replica}" "${cylinder_z_cm}" "${estimated_cGy}" "${mc_err_cGy}" "${mu_err_cGy}" "${det_err_cGy}" "${total_err_cGy}" "${rms_cGy}" \
            >> "${replica_csv}"
    done
done

awk -F',' '
    NR==1 {next}
    {
        z = $2
        count[z] += 1
        dose_sum[z] += ($3 + 0)
        mc_err_sum[z] += ($4 + 0)
        mu_err_sum[z] += ($5 + 0)
        det_err_sum[z] += ($6 + 0)
        total_err_sum[z] += ($7 + 0)
        internal_rms_sum[z] += ($8 + 0)
        dose[z, count[z]] = $3 + 0
        mc_err[z, count[z]] = $4 + 0
        mu_err[z, count[z]] = $5 + 0
        det_err[z, count[z]] = $6 + 0
        total_err[z, count[z]] = $7 + 0
        internal_rms[z, count[z]] = $8 + 0
        if (!(z in seen)) {
            seen[z] = 1
            order[++order_count] = z
        }
    }
    END {
        printf "cylinder_z_cm,depth_cm,mean_estimated_total_absorbed_dose_cGy,stddev_cGy,err_cGy,err_rel_percent,mean_mc_err_cGy,mean_mu_err_cGy,mean_det_err_cGy,mean_total_err_cGy,mean_rms_cGy\n"
        for (oi = 1; oi <= order_count; oi++) {
            z = order[oi]
            n = count[z]
            depth = 10 - z
            mean = dose_sum[z] / n
            mc_err_mean = mc_err_sum[z] / n
            mu_err_mean = mu_err_sum[z] / n
            det_err_mean = det_err_sum[z] / n
            total_err_mean = total_err_sum[z] / n
            internal_rms_mean = internal_rms_sum[z] / n

            dose_var = 0
            if (n > 1) {
                for (i = 1; i <= n; i++) {
                    dose_var += (dose[z, i] - mean) * (dose[z, i] - mean)
                }
                stddev = sqrt(dose_var / (n - 1))
            } else {
                stddev = 0
            }

            err = stddev / sqrt(n)
            err_rel = (mean != 0) ? 100 * err / ((mean < 0) ? -mean : mean) : 0

            printf "%s,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g\n",
                   z, depth, mean, stddev, err, err_rel, mc_err_mean, mu_err_mean, det_err_mean, total_err_mean, internal_rms_mean
        }
    }
' "${replica_csv}" > "${summary_csv}"

{
    echo "PDD summary"
    echo "Run type: ${run_type}"
    echo "Cylinder material: ${cylinder_material}"
    echo "Cylinder radius: ${cylinder_radius_mm} mm"
    echo "Cylinder height: ${cylinder_height_mm} mm"
    echo "Envelope material: ${envelope_material}"
    echo "Envelope thickness: ${envelope_thickness_mm} mm"
    echo "Histories per point: ${n_histories}"
    echo "Number of replicas: ${n_replicas}"
    echo "EOF policy: synthetic"
    echo "${sweep_description}"
    echo
    cat "${summary_csv}"
} > "${summary_txt}"

echo "Replica data written to ${replica_csv}"
echo "Sweep summary written to ${summary_csv}"
echo "Summary written to ${summary_txt}"
