cmake_minimum_required(VERSION 3.0)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(rton-json CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

file(GLOB SOURCES src/*.cpp include/*.hpp lib/*.hpp)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# conan.io for non-mingw
if (NOT MINGW)
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()
endif()

# require nlohmann_json
find_package(nlohmann_json 3.7.0 REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(rton-json ${SOURCES})

target_link_libraries(rton-json PRIVATE nlohmann_json::nlohmann_json)
