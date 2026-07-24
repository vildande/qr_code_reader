add_executable(qrcode_tests
  core_test.cpp
  decoder_test.cpp
  mask_and_path_test.cpp
  segment_header_test.cpp
)

target_link_libraries(qrcode_tests PRIVATE qrcode_core Catch2::Catch2WithMain)
target_compile_options(qrcode_tests PRIVATE
  $<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion>
)
target_compile_definitions(qrcode_tests PRIVATE
  QR_SAMPLE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/.."
  QR_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

add_test(NAME qrcode_tests COMMAND qrcode_tests)
