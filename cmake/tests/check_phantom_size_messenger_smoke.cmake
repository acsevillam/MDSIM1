if(NOT DEFINED EXECUTABLE)
  message(FATAL_ERROR "EXECUTABLE must be defined")
endif()

if(NOT DEFINED WORKING_DIRECTORY)
  message(FATAL_ERROR "WORKING_DIRECTORY must be defined")
endif()

if(NOT DEFINED MACRO_PATH)
  message(FATAL_ERROR "MACRO_PATH must be defined")
endif()

if(NOT DEFINED OUTPUT_GDML)
  message(FATAL_ERROR "OUTPUT_GDML must be defined")
endif()

if(NOT DEFINED EXPECTED_GDML_REGEX)
  message(FATAL_ERROR "EXPECTED_GDML_REGEX must be defined")
endif()

file(MAKE_DIRECTORY "${WORKING_DIRECTORY}/cmake/tests/output")

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
  message(FATAL_ERROR "Phantom size messenger smoke test failed to run.\n${run_output}")
endif()

if(DEFINED EXPECTED_OUTPUT_REGEX AND NOT run_output MATCHES "${EXPECTED_OUTPUT_REGEX}")
  message(FATAL_ERROR
    "Phantom size messenger smoke test did not produce the expected runtime output.\n${run_output}")
endif()

set(output_gdml_path "${WORKING_DIRECTORY}/${OUTPUT_GDML}")
if(NOT EXISTS "${output_gdml_path}")
  message(FATAL_ERROR "Expected GDML export was not written: ${output_gdml_path}")
endif()

file(READ "${output_gdml_path}" exported_gdml)
if(NOT exported_gdml MATCHES "${EXPECTED_GDML_REGEX}")
  message(FATAL_ERROR
    "GDML export does not match the requested phantom size.\nExpected regex: ${EXPECTED_GDML_REGEX}\n${exported_gdml}")
endif()

if(run_output MATCHES
   "Fatal Exception|segmentation fault|Segmentation fault|AddressSanitizer|GeometryExportPhysicalVolumeNotFound")
  message(FATAL_ERROR
    "Phantom size messenger smoke test exposed a runtime crash or missing geometry export.\n${run_output}")
endif()

message(STATUS "Phantom size messenger smoke test passed")
