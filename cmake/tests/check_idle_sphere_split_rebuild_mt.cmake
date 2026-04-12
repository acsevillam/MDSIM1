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
  message(FATAL_ERROR "Idle sphere split rebuild MT test failed to run.\n${run_output}")
endif()

string(REGEX MATCHALL "Sphere Results: sphere\\[0\\]" summary_matches "${run_output}")
list(LENGTH summary_matches summary_count)

if(NOT summary_count EQUAL 3)
  message(FATAL_ERROR
    "Expected sphere[0] to appear in all three detector-specific result blocks during split rebuild under MT.\n${run_output}")
endif()

if(run_output MATCHES "End of Global Run")
  message(FATAL_ERROR
    "Idle sphere split rebuild MT should not emit the legacy global detector summary.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorSphere_water_phys_0")
  message(FATAL_ERROR
    "Expected split-at-interface placement to materialize a water-side sphere volume.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorSphere_air_phys_0")
  message(FATAL_ERROR
    "Expected split-at-interface placement to materialize an air-side sphere volume.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorSphere_air_phys:0")
  message(FATAL_ERROR
    "Expected fully outside placement to rebuild the sphere entirely in world air.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorSphere_phys:0")
  message(FATAL_ERROR
    "Expected fully inside placement to rebuild the sphere back as a single volume.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR
    "Idle sphere split rebuild MT run exposed stale detector runtime state after repeated geometry rebuilds.\n${run_output}")
endif()

message(STATUS "Idle sphere split rebuild MT test passed")
