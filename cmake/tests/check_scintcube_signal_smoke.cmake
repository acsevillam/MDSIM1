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
  message(FATAL_ERROR "scintCube signal smoke test failed to run.\n${run_output}")
endif()

if(NOT run_output MATCHES "ScintCube Results: scintCube\\[0\\]")
  message(FATAL_ERROR "Expected scintCube results block.\n${run_output}")
endif()

if(NOT run_output MATCHES "PhotosensorType: SiPM")
  message(FATAL_ERROR "Expected scintCube results to report the configured SiPM sensor.\n${run_output}")
endif()

if(NOT run_output MATCHES "Events with detected signal: [1-9][0-9]*")
  message(FATAL_ERROR "Expected non-zero detected signal for scintCube.\n${run_output}")
endif()

if(NOT run_output MATCHES "Mean detected photoelectrons per event")
  message(FATAL_ERROR "Expected detected photoelectron summary for scintCube.\n${run_output}")
endif()

if(NOT run_output MATCHES "Estimated absorbed dose in water")
  message(FATAL_ERROR "Expected calibrated dose summary for scintCube.\n${run_output}")
endif()

if(run_output MATCHES "End of Global Run")
  message(FATAL_ERROR "scintCube should not emit a global detector summary anymore.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR "scintCube smoke test exposed stale detector runtime state.\n${run_output}")
endif()

message(STATUS "scintCube signal smoke test passed")
