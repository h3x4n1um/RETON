cmake_minimum_required(VERSION 3.1)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(rton-json CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -pedantic -static -s")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4 -WX -MT")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB SOURCES src/*.cpp include/*.hpp)

# require nlohmann_json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(third_party/json)

# require fifo_map
add_subdirectory(third_party/fifo_map)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable(rton-json ${SOURCES})

target_link_libraries(rton-json PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(rton-json PRIVATE fifo_map::fifo_map)
