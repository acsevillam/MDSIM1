#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../../.." && pwd)"

run_type="CubeCalibrationChargeAir"
material="G4_AIR"
cube_side_mm="5.0"

output_dir="${repo_root}/output"
mkdir -p "$output_dir"

timestamp=$(date +"%Y%m%d_%H%M%S")
job_dir="${output_dir}/${run_type}/${timestamp}"
mkdir -p "$job_dir"

bash "${script_dir}/run_calibration_charge_average.sh" \
  "${run_type}" "${material}" "${cube_side_mm}" "${output_dir}" "${timestamp}" "${job_dir}" \
  > "${job_dir}/calibration_charge_average_air.log"
