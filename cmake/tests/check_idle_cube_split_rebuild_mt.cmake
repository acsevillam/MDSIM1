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
  message(FATAL_ERROR "Idle cube split rebuild MT test failed to run.\n${run_output}")
endif()

string(REGEX MATCHALL "--------------------End of Global Run-----------------------"
       end_of_run_matches "${run_output}")
list(LENGTH end_of_run_matches end_of_run_count)

if(NOT end_of_run_count EQUAL 3)
  message(FATAL_ERROR
    "Expected three completed runs while rebuilding cube split placements in Idle under MT.\n${run_output}")
endif()

string(REGEX MATCHALL "Detector Summary: cube\\[0\\]" cube_summary_matches "${run_output}")
list(LENGTH cube_summary_matches cube_summary_count)

if(NOT cube_summary_count EQUAL 3)
  message(FATAL_ERROR
    "Expected cube[0] to appear in all three runs during split rebuild under MT.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorCube_water_phys_0")
  message(FATAL_ERROR
    "Expected split-at-interface placement to materialize a water-side cube volume.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorCube_air_phys_0")
  message(FATAL_ERROR
    "Expected split-at-interface placement to materialize an air-side cube volume.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorCube_air_phys:0")
  message(FATAL_ERROR
    "Expected fully outside placement to rebuild the cube entirely in world air.\n${run_output}")
endif()

if(NOT run_output MATCHES "DetectorCube_phys:0")
  message(FATAL_ERROR
    "Expected fully inside placement to rebuild the cube back as a single volume.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR
    "Idle cube split rebuild MT run exposed stale detector state after repeated geometry rebuilds.\n${run_output}")
endif()

message(STATUS "Idle cube split rebuild MT test passed")
