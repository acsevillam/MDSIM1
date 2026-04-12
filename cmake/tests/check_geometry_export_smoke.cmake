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
  message(FATAL_ERROR "Geometry export smoke test failed to run.\n${run_output}")
endif()

if(NOT run_output MATCHES "Registered physical volumes:")
  message(FATAL_ERROR "Expected listPhysicalVolumes output.\n${run_output}")
endif()

if(NOT run_output MATCHES " - WaterBox copyNo=0 logical=WaterBox")
  message(FATAL_ERROR "Expected WaterBox physical volume listing with copyNo.\n${run_output}")
endif()

if(NOT run_output MATCHES "Registered logical volumes:")
  message(FATAL_ERROR "Expected listLogicalVolumes output.\n${run_output}")
endif()

if(NOT run_output MATCHES " - WaterBox count=1")
  message(FATAL_ERROR "Expected WaterBox logical volume listing.\n${run_output}")
endif()

set(exported_waterbox "${WORKING_DIRECTORY}/cmake/tests/output/exported_waterbox.gdml")
set(exported_waterbox_logical "${WORKING_DIRECTORY}/cmake/tests/output/exported_waterbox_logical.gdml")
set(exported_model11_root "${WORKING_DIRECTORY}/cmake/tests/output/exported_model11_root.gdml")

foreach(exported_file IN ITEMS "${exported_waterbox}" "${exported_waterbox_logical}" "${exported_model11_root}")
  if(NOT EXISTS "${exported_file}")
    message(FATAL_ERROR "Expected exported GDML file not found: ${exported_file}")
  endif()
endforeach()

function(probe_export exported_file expected_regex expected_aux_regex)
  execute_process(
    COMMAND "${PROBE_EXECUTABLE}" "${exported_file}"
    WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    RESULT_VARIABLE probe_result
    OUTPUT_VARIABLE probe_stdout
    ERROR_VARIABLE probe_stderr
    TIMEOUT 180
  )
  set(probe_output "${probe_stdout}\n${probe_stderr}" PARENT_SCOPE)
  if(NOT probe_result EQUAL 0)
    message(FATAL_ERROR "Failed to probe exported GDML ${exported_file}.\n${probe_stdout}\n${probe_stderr}")
  endif()
  if(NOT probe_stdout MATCHES "world=MD1ExportWorldLogical daughters=1")
    message(FATAL_ERROR "Expected synthetic export world in ${exported_file}.\n${probe_stdout}")
  endif()
  if(NOT probe_stdout MATCHES "${expected_regex}")
    message(FATAL_ERROR "Unexpected exported GDML root for ${exported_file}.\n${probe_stdout}")
  endif()
  if(NOT probe_stdout MATCHES "${expected_aux_regex}")
    message(FATAL_ERROR "Unexpected exported auxiliary metadata for ${exported_file}.\n${probe_stdout}")
  endif()
endfunction()

probe_export("${exported_waterbox}" "daughter=WaterBox copyNo=0 logical=WaterBox" "aux.color=1 aux.sensdet=0")
probe_export("${exported_waterbox_logical}" "daughter=WaterBox copyNo=0 logical=WaterBox" "aux.color=1 aux.sensdet=0")
probe_export("${exported_model11_root}"
             "daughter=ImportedModel11Phys_PassiveAssemblyLV_0_0 copyNo=0"
             "aux.color=[1-9][0-9]* aux.sensdet=[1-9][0-9]*")

message(STATUS "Geometry export smoke test passed")
