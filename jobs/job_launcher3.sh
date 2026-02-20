#!/bin/bash

run_type="BB7Calibration"

# Path to the output folder
output_dir="output"

# Create the output folder if it does not exist
mkdir -p "$output_dir"

timestamp=$(date +"%Y%m%d_%H%M%S")

# Create a results folder for this execution
job_dir="${output_dir}/${run_type}/${timestamp}"
mkdir -p "$job_dir"

# Parameters
nHistories=100000000

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
for ((i=1; i<=${#det_depths[@]}; i++)); do
    for ((j=1; j<=12; j++)); do
        det_depth="${det_depths[i]}"
        result_dir="${job_dir}"/"${det_depth}cm"/"${j}"
        mkdir -p "$result_dir"
        echo "Result directory created: $result_dir"

        # Run the application with the current parameters, including the macro file
        time ./MultiDetector1 "-v" "off" "-b" "on" "-n" ${nHistories} "-m" "input/DetRotX0deg/BB7Calibration"${det_depths[i]}".in" > "$result_dir"/"output.log"

        # Move the results, including the copied macro file, to the corresponding folder
        mv "analysis"/* "$result_dir"
        cp "input/BB7Calibration.in" "$result_dir"/"macro.in"
        echo "Moved output files to result directory: $result_dir"
    done
done

echo "All simulations have been completed."