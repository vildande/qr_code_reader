include(FetchContent)

FetchContent_Declare(
  ZXing
  GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
  GIT_TAG v3.0.2)

FetchContent_MakeAvailable(ZXing)

zxing_add_package_stb()

add_executable(qrcode 
  src/main.cpp
  src/bit_grid.cpp
  src/data_path.cpp
  src/function_patterns.cpp
  src/zxing_grid_reader.cpp
)
target_compile_features(qrcode PRIVATE "cxx_std_${PROJECT_CXX_STANDARD}")

target_include_directories(qrcode PRIVATE include)

target_link_libraries(qrcode PRIVATE ZXing::ZXing stb::stb)
