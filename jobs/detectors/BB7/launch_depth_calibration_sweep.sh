#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../../.." && pwd)"

run_type="BB7Calibration"

# Path to the output folder
output_dir="${repo_root}/output"

# Create the output folder if it does not exist
mkdir -p "$output_dir"

timestamp=$(date +"%Y%m%d_%H%M%S")

# Create a results folder for this execution
job_dir="${output_dir}/${run_type}/${timestamp}"
mkdir -p "$job_dir"

# Parameters
nHistories=100000000
template_macro="${repo_root}/input/detectors/BB7/templates/BB7Calibration.intmp"

if [[ ! -f "${template_macro}" ]]; then
    echo "Template macro not found: ${template_macro}" >&2
    exit 1
fi

# Create a results folder for this execution
det_depths=(
    "0"
    "2"
    "4"
    "6"
    "8"
    "10"
    "15"
    "20"
    "25"
    "30"
    "35"
)

# Run the application with each set of parameters
for ((i=0; i<${#det_depths[@]}; i++)); do
    det_depth="${det_depths[i]}"
    detector_z="-${det_depth}"

    for ((j=1; j<=12; j++)); do
        result_dir="${job_dir}"/"${det_depth}cm"/"${j}"
        mkdir -p "$result_dir"
        echo "Result directory created: $result_dir"

        macro_copy="${result_dir}/macro.in"
        cp "${template_macro}" "${macro_copy}"

        sed "s|\\*\\*DetectorDepth\\*\\*|${detector_z}|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
        sed "s|\\*\\*DetectorAngle\\*\\*|0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
        sed "s|\\*\\*GantryAngle\\*\\*|0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
        sed "s|\\*\\*CollimatorAngle\\*\\*|0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
        sed "s|\\*\\*BeamCentre\\*\\*|0. 0. 100.|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
        sed "s|\\*\\*BeamRot1\\*\\*|1 0 0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
        sed "s|\\*\\*BeamRot2\\*\\*|0 1 0|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"
        sed "s|\\*\\*BeamDirection\\*\\*|0 0 -1|g" "${macro_copy}" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "${macro_copy}"

        # Run the application with the current parameters, including the macro file
        time "${repo_root}/MultiDetector1" "-v" "off" "-b" "on" "-n" "${nHistories}" "-m" "${macro_copy}" > "${result_dir}/output.log"

        # Move the results, including the copied macro file, to the corresponding folder
        if compgen -G "${repo_root}/analysis/*" > /dev/null; then
            mv "${repo_root}/analysis"/* "$result_dir"
        fi
        echo "Moved output files to result directory: $result_dir"
    done
done

echo "All simulations have been completed."
