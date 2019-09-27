cmake_minimum_required(VERSION 3.0)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(rton-json CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()
message(${CMAKE_CXX_COMPILER_ID})

file(GLOB SOURCES src/*.cpp include/*.hpp lib/*.hpp)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# conan.io for non-window
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()
endif()

# require nlohmann_json
find_package(nlohmann_json 3.7.0 REQUIRED)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
	set(ARCHITECTURE "64-bit")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
	set(ARCHITECTURE "32-bit")
endif()
message(${ARCHITECTURE})

# semver
set(VERSION_MAJOR 2)
set(VERSION_MINOR 7)
set(VERSION_PATCH 4)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/version.hpp.in"
  "${PROJECT_BINARY_DIR}/version.hpp"
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${PROJECT_BINARY_DIR}")

add_executable(rton-json ${SOURCES})

target_link_libraries(rton-json PRIVATE nlohmann_json::nlohmann_json)
