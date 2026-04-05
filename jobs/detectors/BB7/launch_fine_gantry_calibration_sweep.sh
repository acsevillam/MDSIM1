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

source "${script_dir}/run_fine_gantry_calibration_sweep.sh" "${run_type}" "${output_dir}" "${timestamp}" "${job_dir}" > "${job_dir}"/fine_gantry_calibration_sweep.log