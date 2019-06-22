cmake_minimum_required(VERSION 3.0)
project(rton-json)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static -O3")

set(SOURCES src/main.cpp src/RTON_number.cpp src/json2rton.cpp src/rton2json.cpp include/json.hpp include/fifo_map.hpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# semver
set(VERSION_MAJOR 2)
set(VERSION_MINOR 6)
set(VERSION_PATCH 2)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/rton-json.h.in"
  "${PROJECT_BINARY_DIR}/rton-json.h"
  )

include_directories(include)
include_directories("${PROJECT_BINARY_DIR}")

add_executable(rton-json ${SOURCES})
