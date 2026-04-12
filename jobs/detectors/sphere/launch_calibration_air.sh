#!/bin/bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../../.." && pwd)"

run_type="SphereCalibrationAir"
material="G4_AIR"
sphere_radius_mm="2.5"
envelope_material="G4_PLEXIGLASS"
envelope_thickness_mm="0.0"

output_dir="${repo_root}/output"
mkdir -p "$output_dir"

timestamp=$(date +"%Y%m%d_%H%M%S")
job_dir="${output_dir}/${run_type}/${timestamp}"
mkdir -p "$job_dir"

bash "${script_dir}/run_calibration.sh" \
  "${run_type}" "${material}" "${sphere_radius_mm}" "${envelope_material}" "${envelope_thickness_mm}" "${output_dir}" "${timestamp}" "${job_dir}" \
  > "${job_dir}/calibration_air.log"
