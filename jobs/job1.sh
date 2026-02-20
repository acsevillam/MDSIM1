#!/bin/bash

# Parameters
nHistories=50000000

# List of phsp filenames corresponding to each set of parameters
phsp_filenames=(
    "beam/Varian_TrueBeam6MV_01"
    "beam/Varian_TrueBeam6MV_02"
    "beam/Varian_TrueBeam6MV_03"
    "beam/Varian_TrueBeam6MV_04"
    "beam/Varian_TrueBeam6MV_05"
    "beam/Varian_TrueBeam6MV_06"
)

# Path to the template macro file
run_type=$1
macro_file="input/${run_type}.intmp"

# Path to the output folder
output_dir=$2

# String to search for in the macro file, with escaped asterisks
search_string="\*\*PhspFileName\*\*"

# Create the output folder if it does not exist
mkdir -p "$output_dir"

# Get the current date and time in the desired format
timestamp=$3
echo "Timestamp for this execution: $timestamp"

# Run the application with each set of parameters
for ((i=1; i<=${#phsp_filenames[@]}; i++)); do
    # Get the current parameters and phsp filename
    phsp_filename="${phsp_filenames[i]}"
    
    echo "Running simulation with filename: $phsp_filename"
    
    # Create a results folder for this execution
    result_dir="$4"/"${i}"
    mkdir -p "$result_dir"
    echo "Result directory created: $result_dir"

    # Copy the macro file to the output folder
    cp "$macro_file" "$output_dir/"
    
    # Get the path of the copied macro file
    macro_copy="${output_dir}/$(basename "$macro_file")"
    
    # Replace the text in the copied macro file
    sed "s|${search_string}|${phsp_filename}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
    
    # Run the application with the current parameters, including the macro file
    time ./MultiDetector1 "-v" "off" "-b" "on" "-n" ${nHistories} "-m" "$macro_copy" > "$result_dir"/"output.log"

    # Move the results, including the copied macro file, to the corresponding folder
    mv "analysis"/* "$result_dir"
    mv "$macro_copy" "$result_dir"/"macro.in"
    echo "Moved output files to result directory: $result_dir"

done

echo "All simulations have been completed."
