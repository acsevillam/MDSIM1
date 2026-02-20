#!/bin/bash

# Parameters
nHistories=50000000

# List of phsp filenames corresponding to each set of parameters
CollimatorAngles=(
    0
    10
    20
    30
    40
    50
    60
    70
    80
    90
)

PhspFileNames=(
    "beam/Varian_TrueBeam6MV_01"
    "beam/Varian_TrueBeam6MV_02"
    "beam/Varian_TrueBeam6MV_03"
    "beam/Varian_TrueBeam6MV_04"
    "beam/Varian_TrueBeam6MV_05"
    "beam/Varian_TrueBeam6MV_06"
)
Jaw1X="5"
Jaw1Y="5"
Jaw2X="5"
Jaw2Y="5"
GantryAngle="0"
DetectorAngle="-90"

# Path to the template macro file
run_type=$1
macro_file="input/${run_type}.intmp"

# Path to the output folder
output_dir=$2


# String to search for in the macro file, with escaped asterisks
search_string="\*\*CollimatorAngle\*\*"

# Constants to search for in the macro file, with escaped asterisks
PhspFileNameKey="\*\*PhspFileName\*\*"
Jaw1XKey="\*\*Jaw1X\*\*"
Jaw1YKey="\*\*Jaw1Y\*\*"
Jaw2XKey="\*\*Jaw2X\*\*"
Jaw2YKey="\*\*Jaw2Y\*\*"
GantryAngleKey="\*\*GantryAngle\*\*"
DetectorAngleKey="\*\*DetectorAngle\*\*"

# Create the output folder if it does not exist
mkdir -p "$output_dir"

# Get the current date and time in the desired format
timestamp=$3
echo "Timestamp for this execution: $timestamp"

# Run the application with each set of parameters
for ((i=1; i<=${#CollimatorAngles[@]}; i++)); do
    for ((j=1; j<=${#PhspFileNames[@]}; j++)); do
        # Get the current parameters and phsp filename
        CollimatorAngle="${CollimatorAngles[i]}"
        PhspFileName="${PhspFileNames[j]}"
        
        echo "Running simulation with collimator angle: $CollimatorAngle deg"
        
        # Create a results folder for this execution
        result_dir="$4"/"${CollimatorAngle}"/"${j}"
        mkdir -p "$result_dir"
        echo "Result directory created: $result_dir"

        # Copy the macro file to the output folder
        cp "$macro_file" "$output_dir/"
        
        # Get the path of the copied macro file
        macro_copy="${output_dir}/$(basename "$macro_file")"
        
        # Replace the text in the copied macro file
        sed "s|${search_string}|${CollimatorAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${PhspFileNameKey}|${PhspFileName}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw1XKey}|${Jaw1X}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw1YKey}|${Jaw1Y}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw2XKey}|${Jaw2X}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw2YKey}|${Jaw2Y}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${GantryAngleKey}|${GantryAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${DetectorAngleKey}|${DetectorAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        
        # Run the application with the current parameters, including the macro file
        time ./MultiDetector1 "-v" "off" "-b" "on" "-n" ${nHistories} "-m" "$macro_copy" > "$result_dir"/"output.log"

        # Move the results, including the copied macro file, to the corresponding folder
        mv "analysis"/* "$result_dir"
        mv "$macro_copy" "$result_dir"/"macro.in"
        echo "Moved output files to result directory: $result_dir"
    done
done

echo "All simulations have been completed."
