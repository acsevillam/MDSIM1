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
  TIMEOUT 60
)

set(run_output "${run_stdout}\n${run_stderr}")

if(run_result EQUAL 0)
  message(FATAL_ERROR "Invalid EOF policy macro unexpectedly succeeded.\n${run_output}")
endif()

if(NOT run_output MATCHES "Candidates are: abort restart stop synthetic")
  message(FATAL_ERROR
    "Invalid EOF policy did not fail with the expected candidate list.\n${run_output}")
endif()

if(run_output MATCHES "IAEA Reader initialized|Total number of particles in file")
  message(FATAL_ERROR
    "Invalid EOF policy should fail before opening PHSP readers.\n${run_output}")
endif()

message(STATUS "Invalid EOF policy test passed")
