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
  message(FATAL_ERROR "Idle model11 split rebuild MT test failed to run.\n${run_output}")
endif()

string(REGEX MATCHALL "Model11 Results: model11\\[0\\]" model11_summary_matches "${run_output}")
list(LENGTH model11_summary_matches model11_summary_count)

if(NOT model11_summary_count EQUAL 3)
  message(FATAL_ERROR
    "Expected model11[0] to appear in all three detector-specific result blocks during split rebuild under MT.\n${run_output}")
endif()

if(NOT run_output MATCHES "PV_LV_Sensor_water copyNo=0 logical=LV_Sensor_water mother=WaterBox")
  message(FATAL_ERROR
    "Expected split-at-interface placement to materialize the water-side model11 sensor fragment under MT.\n${run_output}")
endif()

if(NOT run_output MATCHES "PV_LV_Sensor_air copyNo=0 logical=LV_Sensor_air mother=world_log")
  message(FATAL_ERROR
    "Expected split-at-interface placement to materialize the air-side model11 sensor fragment under MT.\n${run_output}")
endif()

if(NOT run_output MATCHES "PV_LV_Sensor copyNo=0 logical=LV_Sensor mother=world_log")
  message(FATAL_ERROR
    "Expected the fully outside split rebuild to relocate the full model11 sensor into world_log under MT.\n${run_output}")
endif()

if(NOT run_output MATCHES "PV_LV_Sensor copyNo=0 logical=LV_Sensor mother=WaterBox")
  message(FATAL_ERROR
    "Expected the fully inside split rebuild to restore the full model11 sensor under WaterBox under MT.\n${run_output}")
endif()

if(run_output MATCHES "DetectorDigitizerNotFound|DetectorAnalysisNotInitialized|segmentation fault|Segmentation fault|AddressSanitizer")
  message(FATAL_ERROR
    "Idle model11 split rebuild MT run exposed stale detector runtime state after repeated geometry rebuilds.\n${run_output}")
endif()

message(STATUS "Idle model11 split rebuild MT test passed")
