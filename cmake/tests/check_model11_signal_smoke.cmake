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
  message(FATAL_ERROR "model11 signal smoke test failed to run.\n${run_output}")
endif()

if(NOT run_output MATCHES "Model11 Results: model11\\[0\\]")
  message(FATAL_ERROR "Expected model11 results block.\n${run_output}")
endif()

if(NOT run_output MATCHES "PhotosensorType: PMT")
  message(FATAL_ERROR "Expected model11 results to report the default PMT sensor.\n${run_output}")
endif()

if(NOT run_output MATCHES "Imported geometry: enabled root = PassiveAssemblyLV parts = 1 sensitive = 1")
  message(FATAL_ERROR "Expected GDML-driven model11 summary with one imported root subtree and one selected sensitive volume.\n${run_output}")
endif()

if(NOT run_output MATCHES "Events with detected signal: [1-9][0-9]*")
  message(FATAL_ERROR "Expected non-zero detected signal for model11.\n${run_output}")
endif()

if(NOT run_output MATCHES "Mean detected photoelectrons per event")
  message(FATAL_ERROR "Expected detected photoelectron summary for model11.\n${run_output}")
endif()

if(NOT run_output MATCHES "Estimated absorbed dose in water")
  message(FATAL_ERROR "Expected calibrated dose summary for model11.\n${run_output}")
endif()

if(run_output MATCHES "End of Global Run")
  message(FATAL_ERROR "model11 should not emit a global detector summary anymore.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR "model11 smoke test exposed stale detector runtime state.\n${run_output}")
endif()

message(STATUS "model11 signal smoke test passed")
