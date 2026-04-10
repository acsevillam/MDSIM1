#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"
source "${script_dir}/../lib/md1_job_helpers.sh"

if [ "$#" -ne 4 ]; then
    echo "Usage: bash ${BASH_SOURCE[0]} <run_type> <output_dir> <timestamp> <job_dir>" >&2
    exit 1
fi

run_type="$1"
output_dir="$2"
timestamp="$3"
job_dir="$4"

n_histories=200000000
n_replicas=10

scorer_names=(
    "pdd"
    "pddBuildup"
    "latProfile_x"
    "latProfile_y"
    "atIsocenter"
)

append_replica_rows() {
    local scorer_file="$1"
    local replica="$2"
    local replica_csv="$3"

    if [ ! -f "${scorer_file}" ]; then
        echo "Could not find scorer output file: ${scorer_file}" >&2
        exit 1
    fi

    awk -F',' -v replica="${replica}" -v n_histories="${n_histories}" '
        BEGIN { OFS="," }
        /^[[:space:]]*#/ { next }
        NF < 6 { next }
        {
            ix = $1 + 0
            iy = $2 + 0
            iz = $3 + 0
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
                    printf "Negative variance while parsing %s (replica %s, voxel %s,%s,%s): %.12g\n", FILENAME, replica, ix, iy, iz, variance_gy2 > "/dev/stderr"
                    exit 1
                }
                rms_cgy = sqrt(variance_gy2) / 1e-2
            }

            print replica, ix, iy, iz, dose_cgy, rms_cgy, n_entries, total_value_gy, total_val2_gy2
        }
    ' "${scorer_file}" >> "${replica_csv}"
}

write_scorer_summary() {
    local scorer_name="$1"
    local replica_csv="$2"
    local summary_csv="$3"
    local summary_txt="$4"
    local phsp_prefix="$5"

    local axis_label=""
    local axis_mode=""
    local full_span_mm=""
    local n_bins=""

    case "${scorer_name}" in
        pdd)
            axis_label="depth_mm"
            axis_mode="z"
            full_span_mm="500"
            n_bins="125"
            ;;
        pddBuildup)
            axis_label="depth_mm"
            axis_mode="z"
            full_span_mm="24"
            n_bins="12"
            ;;
        latProfile_x)
            axis_label="x_mm"
            axis_mode="x"
            full_span_mm="500"
            n_bins="125"
            ;;
        latProfile_y)
            axis_label="y_mm"
            axis_mode="y"
            full_span_mm="500"
            n_bins="125"
            ;;
        atIsocenter)
            axis_label="position_mm"
            axis_mode="iso"
            full_span_mm="0"
            n_bins="1"
            ;;
        *)
            echo "Unsupported scorer name: ${scorer_name}" >&2
            exit 1
            ;;
    esac

    awk -F',' \
        -v axis_label="${axis_label}" \
        -v axis_mode="${axis_mode}" \
        -v full_span_mm="${full_span_mm}" \
        -v n_bins="${n_bins}" '
        function abs_value(x) {
            return (x < 0) ? -x : x
        }
        function axis_center_mm(ix, iy, iz, step_mm, voxel_index) {
            if (axis_mode == "x") {
                voxel_index = ix + 0
            } else if (axis_mode == "y") {
                voxel_index = iy + 0
            } else if (axis_mode == "z") {
                voxel_index = iz + 0
            } else {
                return 0
            }
            return full_span_mm - ((voxel_index + 0.5) * step_mm)
        }
        NR == 1 { next }
        {
            key = $2 OFS $3 OFS $4
            count[key] += 1
            dose_sum[key] += ($5 + 0)
            rms_sum[key] += ($6 + 0)
            entries_sum[key] += ($7 + 0)

            dose[key, count[key]] = $5 + 0
            rms[key, count[key]] = $6 + 0
            entries[key, count[key]] = $7 + 0

            if (!(key in seen)) {
                seen[key] = 1
                order[++order_count] = key
            }
        }
        END {
            if (order_count == 0) {
                exit 1
            }

            step_mm = (n_bins > 0) ? full_span_mm / n_bins : 0

            printf "iX,iY,iZ,%s,mean_dose_cGy,stddev_dose_cGy,err_dose_cGy,err_rel_percent,mean_internal_rms_cGy,stddev_internal_rms_cGy,err_internal_rms_cGy,mean_entries,stddev_entries,err_entries\n", axis_label
            for (oi = 1; oi <= order_count; oi++) {
                key = order[oi]
                n = count[key]
                split(key, idx, OFS)

                dose_mean = dose_sum[key] / n
                rms_mean = rms_sum[key] / n
                entries_mean = entries_sum[key] / n

                dose_var = 0
                rms_var = 0
                entries_var = 0
                if (n > 1) {
                    for (i = 1; i <= n; i++) {
                        dose_var += (dose[key, i] - dose_mean) * (dose[key, i] - dose_mean)
                        rms_var += (rms[key, i] - rms_mean) * (rms[key, i] - rms_mean)
                        entries_var += (entries[key, i] - entries_mean) * (entries[key, i] - entries_mean)
                    }
                    dose_stddev = sqrt(dose_var / (n - 1))
                    rms_stddev = sqrt(rms_var / (n - 1))
                    entries_stddev = sqrt(entries_var / (n - 1))
                } else {
                    dose_stddev = 0
                    rms_stddev = 0
                    entries_stddev = 0
                }

                dose_err = dose_stddev / sqrt(n)
                rms_err = rms_stddev / sqrt(n)
                entries_err = entries_stddev / sqrt(n)
                dose_err_rel = (dose_mean != 0) ? 100 * dose_err / abs_value(dose_mean) : 0
                axis_value_mm = axis_center_mm(idx[1], idx[2], idx[3], step_mm)

                printf "%s,%s,%s,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g\n", \
                    idx[1], idx[2], idx[3], axis_value_mm, \
                    dose_mean, dose_stddev, dose_err, dose_err_rel, \
                    rms_mean, rms_stddev, rms_err, \
                    entries_mean, entries_stddev, entries_err
            }
        }
    ' "${replica_csv}" > "${summary_csv}"

    {
        echo "${scorer_name} summary"
        echo "Run type: Validation"
        echo "PHSP prefix: ${phsp_prefix}"
        echo "Histories per replica: ${n_histories}"
        echo "Number of replicas: ${n_replicas}"
        echo
        cat "${summary_csv}"
    } > "${summary_txt}"
}

project_root="$(md1_resolve_project_root_containing "${repo_root}" "input/dosimetry/Validation.in")"
macro_file="${project_root}/input/dosimetry/Validation.in"
executable="$(md1_resolve_executable "${project_root}")"
analysis_dir="$(dirname "${executable}")/analysis"
phsp_prefix="$(awk '/\/MultiDetector1\/beamline\/clinac\/phsp\/setPrefix / {print $2; exit}' "${macro_file}")"
scorer_output_files=(
    "pdd.out"
    "pddBuildup.out"
    "latProfile_x.out"
    "latProfile_y.out"
    "atIsocenter.out"
)

if [ -z "${phsp_prefix}" ]; then
    echo "Could not find PHSP prefix in ${macro_file}" >&2
    exit 1
fi

md1_prepare_job_directory "${output_dir}" "${job_dir}"

echo "Timestamp for this execution: ${timestamp}"
echo "Executable: ${executable}"
echo "Macro file: ${macro_file}"
echo "PHSP prefix: ${phsp_prefix}"
echo "Histories per replica: ${n_histories}"
echo "Number of replicas: ${n_replicas}"

for scorer_name in "${scorer_names[@]}"; do
    printf "replica,iX,iY,iZ,dose_cGy,rms_cGy,n_entries,total_value_gy,total_val2_gy2\n" \
        > "${job_dir}/${scorer_name}_by_replica.csv"
done

for ((replica=1; replica<=n_replicas; replica++)); do
    result_dir="${job_dir}/${replica}"
    macro_copy="${result_dir}/macro.in"
    mkdir -p "${result_dir}"

    cp "${macro_file}" "${macro_copy}"

    md1_clear_analysis_outputs "${analysis_dir}" "${scorer_output_files[@]}"

    echo "Running replica ${replica}/${n_replicas}"
    time "${executable}" -m "${macro_copy}" -b on -v off -n "${n_histories}" \
        > "${result_dir}/output.log"

    md1_collect_required_analysis_outputs "${analysis_dir}" "${result_dir}" "${scorer_output_files[@]}"

    for scorer_name in "${scorer_names[@]}"; do
        append_replica_rows \
            "${result_dir}/${scorer_name}.out" \
            "${replica}" \
            "${job_dir}/${scorer_name}_by_replica.csv"
    done
done

for scorer_name in "${scorer_names[@]}"; do
    write_scorer_summary \
        "${scorer_name}" \
        "${job_dir}/${scorer_name}_by_replica.csv" \
        "${job_dir}/${scorer_name}_summary.csv" \
        "${job_dir}/${scorer_name}_summary.txt" \
        "${phsp_prefix}"
done

{
    echo "Validation summary"
    echo "Run type: Validation"
    echo "Timestamp: ${timestamp}"
    echo "Macro file: ${macro_file}"
    echo "PHSP prefix: ${phsp_prefix}"
    echo "Histories per replica: ${n_histories}"
    echo "Number of replicas: ${n_replicas}"
    echo
    echo "Generated summaries:"
    for scorer_name in "${scorer_names[@]}"; do
        echo "${job_dir}/${scorer_name}_summary.csv"
        echo "${job_dir}/${scorer_name}_summary.txt"
    done
} > "${job_dir}/validation_summary.txt"

echo "Validation summary written to ${job_dir}/validation_summary.txt"
echo "All validation simulations have been completed."
