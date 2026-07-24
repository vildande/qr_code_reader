include(FetchContent)

set(ZXING_C_API OFF CACHE BOOL "" FORCE)
set(ZXING_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZXING_WRITERS OFF CACHE STRING "" FORCE)

FetchContent_Declare(
  ZXing
  GIT_REPOSITORY https://github.com/zxing-cpp/zxing-cpp.git
  GIT_TAG v3.0.2)

FetchContent_MakeAvailable(ZXing)

zxing_add_package_stb()

add_library(qrcode_core STATIC
  src/bit_grid.cpp
  src/bit_stream.cpp
  src/data_path.cpp
  src/decoder.cpp
  src/function_patterns.cpp
  src/mask_pattern.cpp
  src/segment_header.cpp
  src/zxing_grid_reader.cpp
)
target_compile_features(qrcode_core PUBLIC "cxx_std_${PROJECT_CXX_STANDARD}")
target_include_directories(qrcode_core PUBLIC include)
target_link_libraries(qrcode_core PUBLIC ZXing::ZXing stb::stb)

add_executable(qrcode src/main.cpp)
target_link_libraries(qrcode PRIVATE qrcode_core)

if(BUILD_TESTING)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.15.1)
  FetchContent_MakeAvailable(Catch2)
  add_subdirectory(test)
endif()
