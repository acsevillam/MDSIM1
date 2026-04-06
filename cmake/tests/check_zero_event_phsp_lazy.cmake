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
  COMMAND "${CMAKE_COMMAND}" -E env G4FORCENUMBEROFTHREADS=2
          "${EXECUTABLE}" -m "${MACRO_PATH}" -v off -b off -n 0
  WORKING_DIRECTORY "${WORKING_DIRECTORY}"
  RESULT_VARIABLE run_result
  OUTPUT_VARIABLE run_stdout
  ERROR_VARIABLE run_stderr
  TIMEOUT 180
)

set(run_output "${run_stdout}\n${run_stderr}")

if(NOT run_result EQUAL 0)
  message(FATAL_ERROR "Zero-event PHSP lazy test failed to run.\n${run_output}")
endif()

if(run_output MATCHES "IAEA Reader initialized|Total number of particles in file")
  message(FATAL_ERROR
    "Zero-event run should not open PHSP readers.\n${run_output}")
endif()

if(NOT run_output MATCHES "Using G4ThreadPool")
  message(FATAL_ERROR
    "Zero-event PHSP lazy test did not reach the MT initialization path.\n${run_output}")
endif()

message(STATUS "Zero-event PHSP lazy initialization test passed")
