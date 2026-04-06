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
  message(FATAL_ERROR "Idle detector refresh test failed to run.\n${run_output}")
endif()

string(REGEX MATCHALL "Detector Summary: BB7\\[0\\]" bb7_summary_matches "${run_output}")
list(LENGTH bb7_summary_matches bb7_summary_count)

if(NOT bb7_summary_count EQUAL 1)
  message(FATAL_ERROR
    "Expected exactly one BB7 detector summary after activating the detector in Idle.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized")
  message(FATAL_ERROR
    "Idle detector refresh run exposed stale detector runtime state.\n${run_output}")
endif()

message(STATUS "Idle detector refresh test passed")
