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
  message(FATAL_ERROR "model11 imported-geometry smoke test failed to run.\n${run_output}")
endif()

if(NOT run_output MATCHES "Model11 Results: model11\\[0\\]")
  message(FATAL_ERROR "Expected model11 results block.\n${run_output}")
endif()

if(NOT run_output MATCHES "Imported geometry: enabled root = PassiveAssemblyLV parts = 1 sensitive = 1")
  message(FATAL_ERROR "Expected imported geometry GDML summary with one imported root subtree and one sensitive volume.\n${run_output}")
endif()

if(NOT run_output MATCHES "Events with detected signal: [1-9][0-9]*")
  message(FATAL_ERROR "Expected non-zero detected signal for imported-geometry model11.\n${run_output}")
endif()

if(run_output MATCHES "Overlap is detected|GeomVol1002|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR "Imported model11 geometry exposed an overlap or runtime failure.\n${run_output}")
endif()

message(STATUS "model11 imported-geometry smoke test passed")
