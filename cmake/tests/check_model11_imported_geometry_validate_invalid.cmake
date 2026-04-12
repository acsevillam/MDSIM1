if(NOT DEFINED EXECUTABLE)
  message(FATAL_ERROR "EXECUTABLE must be defined")
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
  message(FATAL_ERROR "Schema-warning validation smoke should complete even when GDML is invalid.\n${run_output}")
endif()

if(NOT run_output MATCHES "VALIDATION ERROR! attribute 'invalidSchemaAttribute'")
  message(FATAL_ERROR "Expected validation warning for invalidSchemaAttribute.\n${run_output}")
endif()

if(run_output MATCHES "segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR "Schema-warning validation smoke exposed a runtime crash.\n${run_output}")
endif()

message(STATUS "model11 imported-geometry invalid-schema warning test passed")
