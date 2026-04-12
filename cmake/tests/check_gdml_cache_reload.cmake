if(NOT DEFINED PROBE_EXECUTABLE)
  message(FATAL_ERROR "PROBE_EXECUTABLE must be defined")
endif()

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()

if(NOT DEFINED WORKING_DIRECTORY)
  message(FATAL_ERROR "WORKING_DIRECTORY must be defined")
endif()

set(first_gdml "${SOURCE_DIR}/cmake/tests/data/model11/imported_geometry.gdml")
set(second_gdml "${SOURCE_DIR}/cmake/tests/data/model11/imported_geometry_reloaded.gdml")
set(temp_gdml "${WORKING_DIRECTORY}/gdml_cache_probe/reloaded_model11.gdml")

execute_process(
  COMMAND "${PROBE_EXECUTABLE}" "${first_gdml}" "${second_gdml}" "${temp_gdml}"
  WORKING_DIRECTORY "${WORKING_DIRECTORY}"
  RESULT_VARIABLE probe_result
  OUTPUT_VARIABLE probe_stdout
  ERROR_VARIABLE probe_stderr
  TIMEOUT 180
)

set(probe_output "${probe_stdout}\n${probe_stderr}")

if(NOT probe_result EQUAL 0)
  message(FATAL_ERROR "GDML cache reload probe failed.\n${probe_output}")
endif()

if(NOT probe_output MATCHES "GDML cache reload probe passed")
  message(FATAL_ERROR "GDML cache reload probe did not report success.\n${probe_output}")
endif()

message(STATUS "GDML cache reload test passed")
