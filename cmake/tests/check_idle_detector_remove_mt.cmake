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
  COMMAND ${CMAKE_COMMAND} -E env G4FORCENUMBEROFTHREADS=2
          "${EXECUTABLE}" -m "${MACRO_PATH}" -v off -b off -n 0
  WORKING_DIRECTORY "${WORKING_DIRECTORY}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_stdout
  ERROR_VARIABLE run_stderr
  TIMEOUT 180
)

set(run_output "${run_stdout}\n${run_stderr}")

if(NOT run_result EQUAL 0)
  message(FATAL_ERROR "Idle detector remove MT test failed to run.\n${run_output}")
endif()

string(REGEX MATCHALL "BB7 Results: BB7\\[0\\]" bb7_summary_matches "${run_output}")
list(LENGTH bb7_summary_matches bb7_summary_count)

if(NOT bb7_summary_count EQUAL 1)
  message(FATAL_ERROR
    "Expected BB7[0] to appear exactly once under MT in detector-specific results: present before removal and absent after removal.\n${run_output}")
endif()

if(run_output MATCHES "End of Global Run")
  message(FATAL_ERROR
    "Idle detector remove MT should not emit the legacy global detector summary.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR
    "Idle detector remove MT run exposed stale detector state after geometry removal.\n${run_output}")
endif()

message(STATUS "Idle detector remove MT test passed")
