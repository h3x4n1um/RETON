cmake_minimum_required(VERSION 3.0)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(rton-json CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Werror -Wextra -pedantic")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

file(GLOB SOURCES src/*.cpp include/*.hpp lib/*.hpp)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# require nlohmann_json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(third_party/json)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(rton-json ${SOURCES})

target_link_libraries(rton-json PRIVATE nlohmann_json::nlohmann_json)
