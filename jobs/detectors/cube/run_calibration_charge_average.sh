#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../../.." && pwd)"

if [ "$#" -ne 6 ]; then
    echo "Usage: bash ${BASH_SOURCE[0]} <run_type> <cube_material> <cube_side_mm> <output_dir> <timestamp> <job_dir>" >&2
    exit 1
fi

run_type="$1"
cube_material="$2"
cube_side_mm="$3"
output_dir="$4"
timestamp="$5"
job_dir="$6"

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

template_macro="${repo_root}/input/detectors/cube/templates/CubeCalibration.intmp"
if [ ! -f "${template_macro}" ]; then
    echo "Could not find template macro: ${template_macro}" >&2
    exit 1
fi

analysis_dir="$(dirname "${executable}")/analysis"
summary_file="${job_dir}/calculated_total_collected_charge_summary.csv"

mkdir -p "${output_dir}"
mkdir -p "${job_dir}"

echo "Timestamp for this execution: ${timestamp}"
echo "Executable: ${executable}"
echo "Template macro: ${template_macro}"
echo "Cube material: ${cube_material}"
echo "Cube side: ${cube_side_mm} mm"
echo "Histories per replica: ${n_histories}"
echo "Number of replicas: ${n_replicas}"

printf "replica,calculated_total_collected_charge_nC,err_nC,rms_nC\n" > "${summary_file}"

for ((replica=1; replica<=n_replicas; replica++)); do
    result_dir="${job_dir}/${replica}"
    mkdir -p "${result_dir}"

    macro_copy="${result_dir}/macro.in"
    cp "${template_macro}" "${macro_copy}"

    sed "s|\\*\\*CubeSide\\*\\*|${cube_side_mm}|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*CubeMaterial\\*\\*|${cube_material}|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*PhspPrefix\\*\\*|beam/Varian_TrueBeam6MV_|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*Jaw1X\\*\\*|5|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*Jaw2X\\*\\*|5|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*Jaw1Y\\*\\*|5|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*Jaw2Y\\*\\*|5|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*GantryAngle\\*\\*|0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*CollimatorAngle\\*\\*|0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*MU\\*\\*|1|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*WaterBoxZ\\*\\*|-10|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*CubeZ\\*\\*|0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
    sed "s|\\*\\*CubeAngle\\*\\*|0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"

    echo "Running replica ${replica}/${n_replicas}"
    time "${executable}" -m "${macro_copy}" -b on -v off -n "${n_histories}" > "${result_dir}/output.log"

    if [ -d "${analysis_dir}" ]; then
        mv "${analysis_dir}"/* "${result_dir}/" 2>/dev/null || true
    fi

    charge_line=$(awk '
        /^\(7\)  Calculated total collected charge / {print; exit}
    ' "${result_dir}/output.log")

    if [ -z "${charge_line}" ]; then
        echo "Could not find global summary line (7) in ${result_dir}/output.log" >&2
        exit 1
    fi

    charge_nC=$(echo "${charge_line}" | sed -E 's/^.*: ([^ ]+) nC err = .*$/\1/')
    err_nC=$(echo "${charge_line}" | sed -E 's/^.* err = ([^ ]+) nC .*$/\1/')
    rms_nC=$(echo "${charge_line}" | sed -E 's/^.* rms = ([^ ]+) nC$/\1/')

    if [ -z "${charge_nC}" ] || [ -z "${err_nC}" ] || [ -z "${rms_nC}" ]; then
        echo "Could not parse calculated total collected charge line: ${charge_line}" >&2
        exit 1
    fi

    printf "%s,%s,%s,%s\n" "${replica}" "${charge_nC}" "${err_nC}" "${rms_nC}" >> "${summary_file}"
done

awk -F',' '
    NR==1 {next}
    {
        count += 1
        charge[count] = $2 + 0
        internal_err[count] = $3 + 0
        internal_rms[count] = $4 + 0
        charge_sum += charge[count]
        internal_err_sum += internal_err[count]
        internal_rms_sum += internal_rms[count]
    }
    END {
        if (count == 0) {
            exit 1
        }
        charge_mean = charge_sum / count
        internal_err_mean = internal_err_sum / count
        internal_rms_mean = internal_rms_sum / count

        charge_var = 0
        internal_err_var = 0
        internal_rms_var = 0
        if (count > 1) {
            for (i = 1; i <= count; i++) {
                charge_var += (charge[i] - charge_mean) * (charge[i] - charge_mean)
                internal_err_var += (internal_err[i] - internal_err_mean) * (internal_err[i] - internal_err_mean)
                internal_rms_var += (internal_rms[i] - internal_rms_mean) * (internal_rms[i] - internal_rms_mean)
            }
            charge_stddev = sqrt(charge_var / (count - 1))
            internal_err_stddev = sqrt(internal_err_var / (count - 1))
            internal_rms_stddev = sqrt(internal_rms_var / (count - 1))
        } else {
            charge_stddev = 0
            internal_err_stddev = 0
            internal_rms_stddev = 0
        }

        charge_err = charge_stddev / sqrt(count)
        internal_err_of_mean = internal_err_stddev / sqrt(count)
        internal_rms_err = internal_rms_stddev / sqrt(count)
        charge_err_rel = (charge_mean != 0) ? 100 * charge_err / ((charge_mean < 0) ? -charge_mean : charge_mean) : 0
        internal_err_rel = (internal_err_mean != 0) ? 100 * internal_err_of_mean / ((internal_err_mean < 0) ? -internal_err_mean : internal_err_mean) : 0
        internal_rms_rel = (internal_rms_mean != 0) ? 100 * internal_rms_err / ((internal_rms_mean < 0) ? -internal_rms_mean : internal_rms_mean) : 0

        printf "Collected charge over %d replicas:\n", count
        printf "  mean = %.12g nC\n", charge_mean
        printf "  stddev = %.12g nC\n", charge_stddev
        printf "  err = %.12g nC (%.6g %%)\n", charge_err, charge_err_rel
        printf "Per-run internal err over %d replicas:\n", count
        printf "  mean = %.12g nC\n", internal_err_mean
        printf "  stddev = %.12g nC\n", internal_err_stddev
        printf "  err = %.12g nC (%.6g %%)\n", internal_err_of_mean, internal_err_rel
        printf "Per-run internal rms over %d replicas:\n", count
        printf "  mean = %.12g nC\n", internal_rms_mean
        printf "  stddev = %.12g nC\n", internal_rms_stddev
        printf "  err = %.12g nC (%.6g %%)\n", internal_rms_err, internal_rms_rel
    }
' "${summary_file}" | tee "${job_dir}/calculated_total_collected_charge_average.txt"

echo "Summary written to ${summary_file}"
echo "Average report written to ${job_dir}/calculated_total_collected_charge_average.txt"
