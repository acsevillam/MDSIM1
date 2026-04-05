#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../../.." && pwd)"

# Parameters
nHistories=50000000

# List of field sizes (Jaw1X, Jaw1Y, Jaw2X, Jaw2Y)
JawApertures=(
6
)

PhspFileNames=(
    "beam/Varian_TrueBeam6MV_01"
    "beam/Varian_TrueBeam6MV_02"
    "beam/Varian_TrueBeam6MV_03"
    "beam/Varian_TrueBeam6MV_04"
    "beam/Varian_TrueBeam6MV_05"
    "beam/Varian_TrueBeam6MV_06"
)
GantryAngle="0"
CollimatorAngle="0"
DetectorAngle="-90"

# Path to the template macro file
run_type=$1
macro_file="${repo_root}/input/detectors/BB7/templates/${run_type}.intmp"

# Path to the output folder
output_dir=$2


# String to search for in the macro file, with escaped asterisks
search_string1="\*\*Jaw1X\*\*"
search_string2="\*\*Jaw1Y\*\*"
search_string3="\*\*Jaw2X\*\*"
search_string4="\*\*Jaw2Y\*\*"

# Constants to search for in the macro file, with escaped asterisks
PhspFileNameKey="\*\*PhspFileName\*\*"
GantryAngleKey="\*\*GantryAngle\*\*"
CollimatorAngleKey="\*\*CollimatorAngle\*\*"
DetectorAngleKey="\*\*DetectorAngle\*\*"

# Create the output folder if it does not exist
mkdir -p "$output_dir"

# Get the current date and time in the desired format
timestamp=$3
echo "Timestamp for this execution: $timestamp"

# Run the application with each set of parameters
for ((i=1; i<=${#JawApertures[@]}; i++)); do
    for ((j=1; j<=${#PhspFileNames[@]}; j++)); do
        # Get the current parameters and phsp filename
        JawAperture="${JawApertures[i]}"
        num2=2
        FieldSize=$((${JawApertures[i]} * num2))
        PhspFileName="${PhspFileNames[j]}"
        
        echo "Running simulation with field size: ${FieldSize}x${FieldSize} cm"
        
        # Create a results folder for this execution
        result_dir="$4"/"${FieldSize}x${FieldSize}cm"/"${j}"
        mkdir -p "$result_dir"
        echo "Result directory created: $result_dir"

        # Copy the macro file to the output folder
        cp "$macro_file" "$output_dir/"
        
        # Get the path of the copied macro file
        macro_copy="${output_dir}/$(basename "$macro_file")"
        
        # Replace the text in the copied macro file
        sed "s|${search_string1}|${JawAperture}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${search_string2}|${JawAperture}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${search_string3}|${JawAperture}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${search_string4}|${JawAperture}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${PhspFileNameKey}|${PhspFileName}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${GantryAngleKey}|${GantryAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${CollimatorAngleKey}|${CollimatorAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${DetectorAngleKey}|${DetectorAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        
        # Run the application with the current parameters, including the macro file
        time "${repo_root}/MultiDetector1" "-v" "off" "-b" "on" "-n" ${nHistories} "-m" "$macro_copy" > "$result_dir"/"output.log"

        # Move the results, including the copied macro file, to the corresponding folder
        mv "${repo_root}/analysis"/* "$result_dir"
        mv "$macro_copy" "$result_dir"/"macro.in"
        echo "Moved output files to result directory: $result_dir"
    done
done

echo "All simulations have been completed."
