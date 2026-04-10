if(NOT DEFINED CTEST_COMMAND)
  message(FATAL_ERROR "CTEST_COMMAND is required")
endif()

if(NOT DEFINED BUILD_DIR)
  message(FATAL_ERROR "BUILD_DIR is required")
endif()

if(NOT DEFINED LABEL)
  message(FATAL_ERROR "LABEL is required")
endif()

if(NOT DEFINED DESCRIPTION)
  set(DESCRIPTION "${LABEL}")
endif()

execute_process(
  COMMAND "${CTEST_COMMAND}" --test-dir "${BUILD_DIR}" -N -L "${LABEL}"
  RESULT_VARIABLE ctest_list_result
  OUTPUT_VARIABLE ctest_list_output
  ERROR_VARIABLE ctest_list_error
)

if(NOT ctest_list_result EQUAL 0)
  message(FATAL_ERROR
    "Failed to inspect tests for label '${LABEL}'.\n"
    "${ctest_list_output}\n${ctest_list_error}"
  )
endif()

if(ctest_list_output MATCHES "Total Tests: 0")
  message(STATUS "No tests currently labeled '${LABEL}' (${DESCRIPTION}).")
  return()
endif()

execute_process(
  COMMAND "${CTEST_COMMAND}" --test-dir "${BUILD_DIR}" --output-on-failure -L "${LABEL}"
  RESULT_VARIABLE ctest_run_result
)

if(NOT ctest_run_result EQUAL 0)
  message(FATAL_ERROR "CTest failed for label '${LABEL}' (${DESCRIPTION}).")
endif()
