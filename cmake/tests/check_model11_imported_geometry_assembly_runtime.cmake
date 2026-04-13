if(NOT DEFINED EXECUTABLE)
  message(FATAL_ERROR "EXECUTABLE must be defined")
endif()

if(NOT DEFINED PROBE_EXECUTABLE)
  message(FATAL_ERROR "PROBE_EXECUTABLE must be defined")
endif()

if(NOT DEFINED WORKING_DIRECTORY)
  message(FATAL_ERROR "WORKING_DIRECTORY must be defined")
endif()

if(NOT DEFINED MACRO_PATH)
  message(FATAL_ERROR "MACRO_PATH must be defined")
endif()

execute_process(
  COMMAND "${EXECUTABLE}" -m "${MACRO_PATH}" -v off -b off -n 0
  WORKING_DIRECTORY "${WORKING_DIRECTORY}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_stdout
  ERROR_VARIABLE run_stderr
  TIMEOUT 180
)

set(run_output "${run_stdout}\n${run_stderr}")

if(NOT run_result EQUAL 0)
  message(FATAL_ERROR "model11 assembly runtime test failed to run.\n${run_output}")
endif()

if(NOT run_output MATCHES " - PV_LV_Sensor copyNo=0 logical=LV_Sensor mother=WaterBox")
  message(FATAL_ERROR "Expected flat runtime placement for LV_Sensor under WaterBox.\n${run_output}")
endif()

if(NOT run_output MATCHES " - PV_LV_Coupler copyNo=0 logical=LV_Coupler mother=WaterBox")
  message(FATAL_ERROR "Expected flat runtime placement for LV_Coupler under WaterBox.\n${run_output}")
endif()

if(run_output MATCHES "LV_Model11World")
  message(FATAL_ERROR "LV_Model11World should not remain registered in the live geometry.\n${run_output}")
endif()

if(run_output MATCHES "PV_Assembly_Model11")
  message(FATAL_ERROR "PV_Assembly_Model11 should not remain registered in the live geometry.\n${run_output}")
endif()

if(run_output MATCHES "av_1_impr_")
  message(FATAL_ERROR "Flattened model11 runtime should not expose FreeCAD imprint names.\n${run_output}")
endif()

set(exported_sensor "${WORKING_DIRECTORY}/cmake/tests/output/exported_model11_sensor.gdml")
set(exported_coupler "${WORKING_DIRECTORY}/cmake/tests/output/exported_model11_coupler.gdml")
set(exported_waterbox "${WORKING_DIRECTORY}/cmake/tests/output/exported_model11_waterbox.gdml")

foreach(exported_file IN ITEMS "${exported_sensor}" "${exported_coupler}" "${exported_waterbox}")
  if(NOT EXISTS "${exported_file}")
    message(FATAL_ERROR "Expected exported GDML file not found: ${exported_file}")
  endif()
endforeach()

function(probe_export exported_file expected_regex)
  execute_process(
    COMMAND "${PROBE_EXECUTABLE}" "${exported_file}"
    WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    RESULT_VARIABLE probe_result
    OUTPUT_VARIABLE probe_stdout
    ERROR_VARIABLE probe_stderr
    TIMEOUT 180
  )

  if(NOT probe_result EQUAL 0)
    message(FATAL_ERROR "Failed to probe exported GDML ${exported_file}.\n${probe_stdout}\n${probe_stderr}")
  endif()

  if(NOT probe_stdout MATCHES "world=MD1ExportWorldLogical daughters=1")
    message(FATAL_ERROR "Expected synthetic export world in ${exported_file}.\n${probe_stdout}")
  endif()

  if(NOT probe_stdout MATCHES "${expected_regex}")
    message(FATAL_ERROR "Unexpected exported GDML root for ${exported_file}.\n${probe_stdout}")
  endif()
endfunction()

probe_export("${exported_sensor}" "daughter=PV_LV_Sensor copyNo=0 logical=LV_Sensor")
probe_export("${exported_coupler}" "daughter=PV_LV_Coupler copyNo=0 logical=LV_Coupler")

execute_process(
  COMMAND "${PROBE_EXECUTABLE}" "${exported_waterbox}"
  WORKING_DIRECTORY "${WORKING_DIRECTORY}"
  RESULT_VARIABLE waterbox_probe_result
  OUTPUT_VARIABLE waterbox_probe_stdout
  ERROR_VARIABLE waterbox_probe_stderr
  TIMEOUT 180
)

if(NOT waterbox_probe_result EQUAL 0)
  message(FATAL_ERROR "Failed to probe exported WaterBox GDML.\n${waterbox_probe_stdout}\n${waterbox_probe_stderr}")
endif()

if(NOT waterbox_probe_stdout MATCHES "node=MD1ExportWorldLogical/WaterBox/PV_LV_Sensor copyNo=0 logical=LV_Sensor translation_mm=\\(0\\.000,0\\.000,100\\.000\\)")
  message(FATAL_ERROR "Expected LV_Sensor runtime placement to follow the stored detector transform after translateTo + rotateX.\n${waterbox_probe_stdout}")
endif()

if(NOT waterbox_probe_stdout MATCHES "node=MD1ExportWorldLogical/WaterBox/PV_LV_Coupler copyNo=0 logical=LV_Coupler translation_mm=\\(0\\.000,6\\.000,100\\.000\\)")
  message(FATAL_ERROR "Expected LV_Coupler to preserve the 6 mm assembly offset after rotateX.\n${waterbox_probe_stdout}")
endif()

message(STATUS "model11 assembly runtime test passed")
