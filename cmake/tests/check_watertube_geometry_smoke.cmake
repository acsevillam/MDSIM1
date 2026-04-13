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
  message(FATAL_ERROR "waterTube geometry smoke test failed to run.\n${run_output}")
endif()

if(NOT run_output MATCHES "Checking overlaps for volume WaterTube:0")
  message(FATAL_ERROR
    "Expected the selected waterTube phantom to be built and checked for overlaps.\n${run_output}")
endif()

if(NOT run_output MATCHES "Checking overlaps for volume DetectorCube_phys:0")
  message(FATAL_ERROR
    "Expected the cube detector to be placed inside the active phantom.\n${run_output}")
endif()

if(NOT run_output MATCHES "Cube Results: cube\\[0\\]")
  message(FATAL_ERROR
    "Expected the cube detector module to complete the run and print its results.\n${run_output}")
endif()

if(run_output MATCHES
   "Fatal Exception|segmentation fault|Segmentation fault|AddressSanitizer|DetectorDigitizerNotFound|DetectorAnalysisNotInitialized")
  message(FATAL_ERROR
    "waterTube geometry smoke test exposed a runtime crash or stale detector state.\n${run_output}")
endif()

message(STATUS "waterTube geometry smoke test passed")
