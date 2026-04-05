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

n_histories=300000000
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
summary_file="${job_dir}/dmax_summary.csv"

mkdir -p "$output_dir"
mkdir -p "$job_dir"

echo "Timestamp for this execution: ${timestamp}"
echo "Executable: ${executable}"
echo "Macro file: ${macro_file}"
echo "Histories per replica: ${n_histories}"
echo "Number of replicas: ${n_replicas}"

printf "replica,dose_at_dmax_cGy,rms_cGy,n_entries\n" > "${summary_file}"

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

    depth_line=$(awk '
        /------------------ Dose atDepth1\.4cm ---------------------/ {getline; print; exit}
    ' "${result_dir}/output.log")

    if [ -z "${depth_line}" ]; then
        echo "Could not find Dose atDepth1.4cm block in ${result_dir}/output.log" >&2
        exit 1
    fi

    dose_cgy=$(echo "${depth_line}" | awk '{print $2}')
    rms_cgy=$(echo "${depth_line}" | awk '{print $6}')
    n_entries=$(echo "${depth_line}" | awk '{for(i=1;i<=NF;i++) if($i=="nEntries") {print $(i+2); exit}}')

    if [ -z "${n_entries}" ]; then
        echo "Could not parse nEntries from: ${depth_line}" >&2
        exit 1
    fi

    printf "%s,%s,%s,%s\n" "${replica}" "${dose_cgy}" "${rms_cgy}" "${n_entries}" >> "${summary_file}"
done

awk -F',' '
    NR==1 {next}
    {
        dose_sum += $2
        rms_sum += $3
        entries_sum += $4
        count += 1
    }
    END {
        if (count == 0) {
            exit 1
        }
        printf "Average Dose at dmax (Depth1.4cm) over %d replicas: %.12g cGy\n", count, dose_sum / count
        printf "Average RMS at dmax (Depth1.4cm) over %d replicas: %.12g cGy\n", count, rms_sum / count
        printf "Average nEntries at dmax (Depth1.4cm) over %d replicas: %.12g\n", count, entries_sum / count
    }
' "${summary_file}" | tee "${job_dir}/dmax_average.txt"

echo "Summary written to ${summary_file}"
echo "Average report written to ${job_dir}/dmax_average.txt"
