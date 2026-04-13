if(NOT DEFINED PROBE_EXECUTABLE)
  message(FATAL_ERROR "PROBE_EXECUTABLE must be defined")
endif()

function(assert_probe value expected_regex)
  execute_process(
    COMMAND "${PROBE_EXECUTABLE}" "${value}"
    RESULT_VARIABLE probe_result
    OUTPUT_VARIABLE probe_stdout
    ERROR_VARIABLE probe_stderr
    TIMEOUT 30
  )

  if(NOT probe_result EQUAL 0)
    message(FATAL_ERROR "Color codec probe failed for ${value}.\n${probe_stdout}\n${probe_stderr}")
  endif()

  if(NOT probe_stdout MATCHES "${expected_regex}")
    message(FATAL_ERROR "Unexpected color codec output for ${value}.\n${probe_stdout}")
  endif()
endfunction()

assert_probe("#336699" "alpha=1\\.000000 encoded=#33669900")
assert_probe("#ff260000" "alpha=1\\.000000 encoded=#ff260000")
assert_probe("#0433ffb2" "alpha=0\\.301961 encoded=#0433ffb2")
assert_probe("Blue" "alpha=1\\.000000 encoded=#0000ff00")

execute_process(
  COMMAND "${PROBE_EXECUTABLE}" "#1234567g"
  RESULT_VARIABLE invalid_result
  OUTPUT_VARIABLE invalid_stdout
  ERROR_VARIABLE invalid_stderr
  TIMEOUT 30
)

if(invalid_result EQUAL 0)
  message(FATAL_ERROR "Expected invalid FreeCAD color to fail parsing.\n${invalid_stdout}")
endif()

set(invalid_output "${invalid_stdout}\n${invalid_stderr}")
if(NOT invalid_output MATCHES "error=")
  message(FATAL_ERROR "Expected invalid color probe to report an error.\n${invalid_output}")
endif()

message(STATUS "GDML color codec probe passed")
