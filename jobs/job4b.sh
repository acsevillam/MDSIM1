#!/bin/bash

# Parameters
nHistories=100000000

# List of phsp filenames corresponding to each set of parameters
GantryAngles=(
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

GantryAnglesDummy=(
    90
    100
    110
    120
    130
    140
    150
    160
    170
    180
    )

BeamCentres=(
    "0. 0. 100." # 0
    "17.365 0 98.481" # 10
    "34.202 0 93.969" # 20
    "50.000 0.000 86.603" # 30
    "64.279	0.000 76.604" # 40
    "76.604	0.000 64.279" # 50
    "86.603	0.000 50.000" # 60
    "93.969	0.000 34.202" # 70
    "98.481	0.000 17.365" # 80
    "100. 0. 0." # 90
    )

BeamRots1=(
    "1 0 0"  # 0
    "0.985 0.000 -0.174"  # 10
    "0.940 0.000 -0.342"  # 20
    "0.866 0.000 -0.500"  # 30
    "0.766 0.000 -0.643"  # 40
    "0.643 0.000 -0.766"  # 50
    "0.500 0.000 -0.866"  # 60
    "0.342 0.000 -0.940"  # 70
    "0.174 0.000 -0.985"  # 80
    "0 0 -1"  # 0
    )
BeamRots2=(
    "0 1 0" # 0
    "0 1 0" # 10
    "0 1 0" # 20
    "0 1 0" # 30
    "0 1 0" # 40
    "0 1 0" # 50
    "0 1 0" # 60
    "0 1 0" # 70
    "0 1 0" # 80
    "0 1 0" # 90
    )
BeamDirections=(
    "0 0 -1" # 0
    "-0.174 0.000 -0.985" # 10
    "-0.342	0.000 -0.940" # 20
    "-0.500	0.000 -0.866" # 30
    "-0.643	0.000 -0.766" # 40
    "-0.766	0.000 -0.643" # 50
    "-0.866	0.000 -0.500" # 60
    "-0.940	0.000 -0.342" # 70
    "-0.985	0.000 -0.174" # 80
    "-1 0 0" # 90
    )

PhspFileName="beam/Varian_TrueBeam6MV_01"
Jaw1X="5"
Jaw1Y="5"
Jaw2X="5"
Jaw2Y="5"
CollimatorAngle="0"
DetectorAngle="-90" # For vertical position set equal to -90 deg

# Path to the template macro file
run_type=$1
macro_file="input/${run_type}.intmp"

# Path to the output folder
output_dir=$2


# String to search for in the macro file, with escaped asterisks
search_string="\*\*GantryAngle\*\*"

# Constants to search for in the macro file, with escaped asterisks
PhspFileNameKey="\*\*PhspFileName\*\*"
Jaw1XKey="\*\*Jaw1X\*\*"
Jaw1YKey="\*\*Jaw1Y\*\*"
Jaw2XKey="\*\*Jaw2X\*\*"
Jaw2YKey="\*\*Jaw2Y\*\*"
CollimatorAngleKey="\*\*CollimatorAngle\*\*"
DetectorAngleKey="\*\*DetectorAngle\*\*"
BeamCentreKey="\*\*BeamCentre\*\*"
BeamRot1Key="\*\*BeamRot1\*\*"
BeamRot2Key="\*\*BeamRot2\*\*"
BeamDirectionKey="\*\*BeamDirection\*\*"

# Create the output folder if it does not exist
mkdir -p "$output_dir"

# Get the current date and time in the desired format
timestamp=$3
echo "Timestamp for this execution: $timestamp"

nRuns=12 

# Run the application with each set of parameters
for ((i=1; i<=${#GantryAngles[@]}; i++)); do
    for ((j=1; j<=nRuns; j++)); do
        # Get the current parameters and phsp filename
        GantryAngle="${GantryAngles[i]}"
        GantryAngleDummy="${GantryAnglesDummy[i]}"
        BeamRot1="${BeamRots1[i]}"
        BeamRot2="${BeamRots2[i]}"
        BeamDirection="${BeamDirections[i]}"
        BeamCentre="${BeamCentres[i]}"
        
        echo "Running simulation with gantry angle: $GantryAngle deg"
        
        # Create a results folder for this execution
        result_dir="$4"/"${GantryAngle}"/"${j}"
        mkdir -p "$result_dir"
        echo "Result directory created: $result_dir"

        # Copy the macro file to the output folder
        cp "$macro_file" "$output_dir/"
        
        # Get the path of the copied macro file
        macro_copy="${output_dir}/$(basename "$macro_file")"
        
        # Replace the text in the copied macro file
        sed "s|${search_string}|${GantryAngleDummy}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${PhspFileNameKey}|${PhspFileName}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw1XKey}|${Jaw1X}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw1YKey}|${Jaw1Y}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw2XKey}|${Jaw2X}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${Jaw2YKey}|${Jaw2Y}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${CollimatorAngleKey}|${CollimatorAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${DetectorAngleKey}|${DetectorAngle}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${BeamCentreKey}|${BeamCentre}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${BeamRot1Key}|${BeamRot1}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${BeamRot2Key}|${BeamRot2}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        sed "s|${BeamDirectionKey}|${BeamDirection}|g" "$macro_copy" > "${macro_copy}.tmp" && mv "${macro_copy}.tmp" "$macro_copy"
        
        # Run the application with the current parameters, including the macro file
        time ./MultiDetector1 "-v" "off" "-b" "on" "-n" ${nHistories} "-m" "$macro_copy" > "$result_dir"/"output.log"

        # Move the results, including the copied macro file, to the corresponding folder
        mv "analysis"/* "$result_dir"
        mv "$macro_copy" "$result_dir"/"macro.in"
        echo "Moved output files to result directory: $result_dir"
    done
done

echo "All simulations have been completed."
