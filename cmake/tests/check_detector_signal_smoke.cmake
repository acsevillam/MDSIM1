if(NOT DEFINED EXECUTABLE)
  message(FATAL_ERROR "EXECUTABLE must be defined")
endif()

if(NOT DEFINED WORKING_DIRECTORY)
  message(FATAL_ERROR "WORKING_DIRECTORY must be defined")
endif()

if(NOT DEFINED MACRO_PATH)
  message(FATAL_ERROR "MACRO_PATH must be defined")
endif()

if(NOT DEFINED DETECTOR_SUMMARY_LABEL)
  message(FATAL_ERROR "DETECTOR_SUMMARY_LABEL must be defined")
endif()

if(NOT DEFINED DETECTOR_NAME)
  message(FATAL_ERROR "DETECTOR_NAME must be defined")
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
  message(FATAL_ERROR "${DETECTOR_NAME} signal smoke test failed to run.\n${run_output}")
endif()

if(NOT run_output MATCHES "Detector Summary: ${DETECTOR_SUMMARY_LABEL}")
  message(FATAL_ERROR
    "Expected detector summary for ${DETECTOR_NAME}.\n${run_output}")
endif()

if(NOT run_output MATCHES "Events with signal: [1-9][0-9]*")
  message(FATAL_ERROR
    "Expected non-zero detector signal for ${DETECTOR_NAME}.\n${run_output}")
endif()

if(NOT run_output MATCHES "Calculated total collected charge")
  message(FATAL_ERROR
    "Expected collected charge summary for ${DETECTOR_NAME}.\n${run_output}")
endif()

if(NOT run_output MATCHES "Estimated total absorbed dose in water")
  message(FATAL_ERROR
    "Expected estimated dose-to-water summary for ${DETECTOR_NAME}.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR
    "Signal smoke test for ${DETECTOR_NAME} exposed stale detector runtime state.\n${run_output}")
endif()

message(STATUS "${DETECTOR_NAME} signal smoke test passed")
