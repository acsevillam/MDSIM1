#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"

if [ "$#" -ne 4 ]; then
    echo "Usage: bash ${BASH_SOURCE[0]} <run_type> <output_dir> <timestamp> <job_dir>" >&2
    exit 1
fi

run_type="$1"
output_dir="$2"
timestamp="$3"
job_dir="$4"

n_histories=200000000
n_replicas=5

if [ -x "${repo_root}/MultiDetector1" ]; then
    executable="${repo_root}/MultiDetector1"
elif [ -x "${repo_root}/build/MultiDetector1" ]; then
    executable="${repo_root}/build/MultiDetector1"
else
    echo "Could not find MultiDetector1 executable under ${repo_root} or ${repo_root}/build" >&2
    exit 1
fi

if [ -f "${repo_root}/input/dosimetry/Calibration.in" ]; then
    macro_file="${repo_root}/input/dosimetry/Calibration.in"
elif [ -f "$(cd "${repo_root}/.." && pwd)/MDSIM1/input/dosimetry/Calibration.in" ]; then
    repo_root="$(cd "${repo_root}/.." && pwd)/MDSIM1"
    macro_file="${repo_root}/input/dosimetry/Calibration.in"
else
    echo "Could not find input/dosimetry/Calibration.in starting from ${repo_root}" >&2
    exit 1
fi

analysis_dir="$(dirname "${executable}")/analysis"
summary_file="${job_dir}/dose_depth_summary.csv"

mkdir -p "$output_dir"
mkdir -p "$job_dir"

echo "Timestamp for this execution: ${timestamp}"
echo "Executable: ${executable}"
echo "Macro file: ${macro_file}"
echo "Histories per replica: ${n_histories}"
echo "Number of replicas: ${n_replicas}"

printf "replica,dose_at_dmax_cGy,rms_at_dmax_cGy,n_entries_at_dmax,dose_at_10cm_cGy,rms_at_10cm_cGy,n_entries_at_10cm\n" > "${summary_file}"

for ((replica=1; replica<=n_replicas; replica++)); do
    result_dir="${job_dir}/${replica}"
    mkdir -p "${result_dir}"

    cp "${macro_file}" "${result_dir}/macro.in"

    echo "Running replica ${replica}/${n_replicas}"
    time "${executable}" -m "${result_dir}/macro.in" -b on -v off -n "${n_histories}" \
        > "${result_dir}/output.log"

    if [ -d "${analysis_dir}" ]; then
        mv "${analysis_dir}"/* "${result_dir}/" 2>/dev/null || true
    fi

    dmax_line=$(awk '
        /------------------ Dose atDepth1\.4cm ---------------------/ {getline; print; exit}
    ' "${result_dir}/output.log")

    if [ -z "${dmax_line}" ]; then
        echo "Could not find Dose atDepth1.4cm block in ${result_dir}/output.log" >&2
        exit 1
    fi

    depth10_line=$(awk '
        /------------------ Dose atDepth10cm ---------------------/ {getline; print; exit}
    ' "${result_dir}/output.log")

    if [ -z "${depth10_line}" ]; then
        echo "Could not find Dose atDepth10cm block in ${result_dir}/output.log" >&2
        exit 1
    fi

    dose_at_dmax_cgy=$(echo "${dmax_line}" | awk '{print $2}')
    rms_at_dmax_cgy=$(echo "${dmax_line}" | awk '{print $6}')
    n_entries_at_dmax=$(echo "${dmax_line}" | awk '{for(i=1;i<=NF;i++) if($i=="nEntries") {print $(i+2); exit}}')

    dose_at_10cm_cgy=$(echo "${depth10_line}" | awk '{print $2}')
    rms_at_10cm_cgy=$(echo "${depth10_line}" | awk '{print $6}')
    n_entries_at_10cm=$(echo "${depth10_line}" | awk '{for(i=1;i<=NF;i++) if($i=="nEntries") {print $(i+2); exit}}')

    if [ -z "${n_entries_at_dmax}" ]; then
        echo "Could not parse nEntries from: ${dmax_line}" >&2
        exit 1
    fi

    if [ -z "${n_entries_at_10cm}" ]; then
        echo "Could not parse nEntries from: ${depth10_line}" >&2
        exit 1
    fi

    printf "%s,%s,%s,%s,%s,%s,%s\n" \
        "${replica}" \
        "${dose_at_dmax_cgy}" \
        "${rms_at_dmax_cgy}" \
        "${n_entries_at_dmax}" \
        "${dose_at_10cm_cgy}" \
        "${rms_at_10cm_cgy}" \
        "${n_entries_at_10cm}" >> "${summary_file}"
done

awk -F',' '
    NR==1 {next}
    {
        count += 1
        dmax_dose[count] = $2 + 0
        dmax_rms[count] = $3 + 0
        dmax_entries[count] = $4 + 0
        depth10_dose[count] = $5 + 0
        depth10_rms[count] = $6 + 0
        depth10_entries[count] = $7 + 0

        dmax_dose_sum += dmax_dose[count]
        dmax_rms_sum += dmax_rms[count]
        dmax_entries_sum += dmax_entries[count]
        depth10_dose_sum += depth10_dose[count]
        depth10_rms_sum += depth10_rms[count]
        depth10_entries_sum += depth10_entries[count]
    }
    END {
        if (count == 0) {
            exit 1
        }
        dmax_dose_mean = dmax_dose_sum / count
        dmax_rms_mean = dmax_rms_sum / count
        dmax_entries_mean = dmax_entries_sum / count
        depth10_dose_mean = depth10_dose_sum / count
        depth10_rms_mean = depth10_rms_sum / count
        depth10_entries_mean = depth10_entries_sum / count

        dmax_dose_var = 0
        dmax_rms_var = 0
        dmax_entries_var = 0
        depth10_dose_var = 0
        depth10_rms_var = 0
        depth10_entries_var = 0
        if (count > 1) {
            for (i = 1; i <= count; i++) {
                dmax_dose_var += (dmax_dose[i] - dmax_dose_mean) * (dmax_dose[i] - dmax_dose_mean)
                dmax_rms_var += (dmax_rms[i] - dmax_rms_mean) * (dmax_rms[i] - dmax_rms_mean)
                dmax_entries_var += (dmax_entries[i] - dmax_entries_mean) * (dmax_entries[i] - dmax_entries_mean)
                depth10_dose_var += (depth10_dose[i] - depth10_dose_mean) * (depth10_dose[i] - depth10_dose_mean)
                depth10_rms_var += (depth10_rms[i] - depth10_rms_mean) * (depth10_rms[i] - depth10_rms_mean)
                depth10_entries_var += (depth10_entries[i] - depth10_entries_mean) * (depth10_entries[i] - depth10_entries_mean)
            }
            dmax_dose_stddev = sqrt(dmax_dose_var / (count - 1))
            dmax_rms_stddev = sqrt(dmax_rms_var / (count - 1))
            dmax_entries_stddev = sqrt(dmax_entries_var / (count - 1))
            depth10_dose_stddev = sqrt(depth10_dose_var / (count - 1))
            depth10_rms_stddev = sqrt(depth10_rms_var / (count - 1))
            depth10_entries_stddev = sqrt(depth10_entries_var / (count - 1))
        } else {
            dmax_dose_stddev = 0
            dmax_rms_stddev = 0
            dmax_entries_stddev = 0
            depth10_dose_stddev = 0
            depth10_rms_stddev = 0
            depth10_entries_stddev = 0
        }

        dmax_dose_err = dmax_dose_stddev / sqrt(count)
        dmax_rms_err = dmax_rms_stddev / sqrt(count)
        dmax_entries_err = dmax_entries_stddev / sqrt(count)
        depth10_dose_err = depth10_dose_stddev / sqrt(count)
        depth10_rms_err = depth10_rms_stddev / sqrt(count)
        depth10_entries_err = depth10_entries_stddev / sqrt(count)

        dmax_dose_err_rel = (dmax_dose_mean != 0) ? 100 * dmax_dose_err / ((dmax_dose_mean < 0) ? -dmax_dose_mean : dmax_dose_mean) : 0
        dmax_rms_err_rel = (dmax_rms_mean != 0) ? 100 * dmax_rms_err / ((dmax_rms_mean < 0) ? -dmax_rms_mean : dmax_rms_mean) : 0
        dmax_entries_err_rel = (dmax_entries_mean != 0) ? 100 * dmax_entries_err / ((dmax_entries_mean < 0) ? -dmax_entries_mean : dmax_entries_mean) : 0
        depth10_dose_err_rel = (depth10_dose_mean != 0) ? 100 * depth10_dose_err / ((depth10_dose_mean < 0) ? -depth10_dose_mean : depth10_dose_mean) : 0
        depth10_rms_err_rel = (depth10_rms_mean != 0) ? 100 * depth10_rms_err / ((depth10_rms_mean < 0) ? -depth10_rms_mean : depth10_rms_mean) : 0
        depth10_entries_err_rel = (depth10_entries_mean != 0) ? 100 * depth10_entries_err / ((depth10_entries_mean < 0) ? -depth10_entries_mean : depth10_entries_mean) : 0

        printf "Dose at dmax (Depth1.4cm) over %d replicas:\n", count
        printf "  mean = %.12g cGy\n", dmax_dose_mean
        printf "  stddev = %.12g cGy\n", dmax_dose_stddev
        printf "  err = %.12g cGy (%.6g %%)\n", dmax_dose_err, dmax_dose_err_rel
        printf "Per-run internal RMS at dmax over %d replicas:\n", count
        printf "  mean = %.12g cGy\n", dmax_rms_mean
        printf "  stddev = %.12g cGy\n", dmax_rms_stddev
        printf "  err = %.12g cGy (%.6g %%)\n", dmax_rms_err, dmax_rms_err_rel
        printf "Per-run internal entries at dmax over %d replicas:\n", count
        printf "  mean = %.12g\n", dmax_entries_mean
        printf "  stddev = %.12g\n", dmax_entries_stddev
        printf "  err = %.12g (%.6g %%)\n", dmax_entries_err, dmax_entries_err_rel
        printf "\n"
        printf "Dose at depth 10 cm over %d replicas:\n", count
        printf "  mean = %.12g cGy\n", depth10_dose_mean
        printf "  stddev = %.12g cGy\n", depth10_dose_stddev
        printf "  err = %.12g cGy (%.6g %%)\n", depth10_dose_err, depth10_dose_err_rel
        printf "Per-run internal RMS at depth 10 cm over %d replicas:\n", count
        printf "  mean = %.12g cGy\n", depth10_rms_mean
        printf "  stddev = %.12g cGy\n", depth10_rms_stddev
        printf "  err = %.12g cGy (%.6g %%)\n", depth10_rms_err, depth10_rms_err_rel
        printf "Per-run internal entries at depth 10 cm over %d replicas:\n", count
        printf "  mean = %.12g\n", depth10_entries_mean
        printf "  stddev = %.12g\n", depth10_entries_stddev
        printf "  err = %.12g (%.6g %%)\n", depth10_entries_err, depth10_entries_err_rel
    }
' "${summary_file}" | tee "${job_dir}/dose_depth_average.txt"

echo "Summary written to ${summary_file}"
echo "Average report written to ${job_dir}/dose_depth_average.txt"
