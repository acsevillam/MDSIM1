if(NOT DEFINED EXECUTABLE)
  message(FATAL_ERROR "EXECUTABLE must be defined")
endif()

if(NOT DEFINED WORKING_DIRECTORY)
  message(FATAL_ERROR "WORKING_DIRECTORY must be defined")
endif()

set(TEST_DIRECTORY "${WORKING_DIRECTORY}/cmake_test_incomplete_phsp_dataset")
file(REMOVE_RECURSE "${TEST_DIRECTORY}")
file(MAKE_DIRECTORY "${TEST_DIRECTORY}/beam")

file(WRITE "${TEST_DIRECTORY}/beam/Broken_01.IAEAphsp" "placeholder")
file(WRITE "${TEST_DIRECTORY}/incomplete_phsp.mac"
"/control/verbose 0\n/run/verbose 0\n/MultiDetector1/beamline/clinac/phsp/clearFiles\n/MultiDetector1/beamline/clinac/phsp/setPrefix beam/Broken_\n")

execute_process(
  COMMAND "${EXECUTABLE}" -m "${TEST_DIRECTORY}/incomplete_phsp.mac" -v off -b off -n 0
  WORKING_DIRECTORY "${TEST_DIRECTORY}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_stdout
  ERROR_VARIABLE run_stderr
  TIMEOUT 60
)

set(run_output "${run_stdout}\n${run_stderr}")

if(run_result EQUAL 0)
  message(FATAL_ERROR "Incomplete PHSP dataset macro unexpectedly succeeded.\n${run_output}")
endif()

if(NOT run_output MATCHES "Incomplete phase-space dataset")
  message(FATAL_ERROR
    "Incomplete PHSP dataset did not fail with the expected early validation message.\n${run_output}")
endif()

if(run_output MATCHES "Could not open IAEA source file to read|IAEA Reader initialized")
  message(FATAL_ERROR
    "Incomplete PHSP dataset should fail before opening PHSP readers.\n${run_output}")
endif()

message(STATUS "Incomplete PHSP dataset validation test passed")
