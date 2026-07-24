configure_file(Doxyfile.in Doxyfile @ONLY)

add_custom_target(doc
  COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  BYPRODUCTS "${CMAKE_BINARY_DIR}/docs/html/index.html"
  COMMENT "Generating QR Code API documentation"
  VERBATIM
)
