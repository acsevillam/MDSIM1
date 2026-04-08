#!/bin/bash

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../../.." && pwd)"

run_type="CubeCalibrationWaterPmma2p5mm"
material="G4_WATER"
cube_side_mm="5.0"
envelope_material="G4_PLEXIGLASS"
envelope_thickness_mm="2.5"

output_dir="${repo_root}/output"
mkdir -p "$output_dir"

timestamp=$(date +"%Y%m%d_%H%M%S")
job_dir="${output_dir}/${run_type}/${timestamp}"
mkdir -p "$job_dir"

bash "${script_dir}/run_calibration.sh" \
  "${run_type}" "${material}" "${cube_side_mm}" "${envelope_material}" "${envelope_thickness_mm}" "${output_dir}" "${timestamp}" "${job_dir}" \
  > "${job_dir}/calibration_water_pmma_2p5mm.log"
