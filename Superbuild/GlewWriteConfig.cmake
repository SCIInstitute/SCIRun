
if(NOT EXISTS "${TEMPLATE}")
  message(FATAL_ERROR "GLEW template not found at: ${TEMPLATE}")
endif()
configure_file("${TEMPLATE}" "${OUT_FILE}" @ONLY)
