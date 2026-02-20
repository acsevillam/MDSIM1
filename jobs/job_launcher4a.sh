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

source ./jobs/job4a.sh "${run_type}" "${output_dir}" "${timestamp}" "${job_dir}" > "${job_dir}"/job4a.log