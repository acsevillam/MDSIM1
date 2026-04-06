#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"

run_type="Calibration"

output_dir="${repo_root}/output"
mkdir -p "$output_dir"

timestamp=$(date +"%Y%m%d_%H%M%S")
job_dir="${output_dir}/${run_type}/${timestamp}"
mkdir -p "$job_dir"

bash "${script_dir}/run_calibration_dmax_average.sh" "${run_type}" "${output_dir}" "${timestamp}" "${job_dir}" \
  > "${job_dir}/calibration_dose_depth_average.log"
