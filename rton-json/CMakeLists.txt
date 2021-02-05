cmake_minimum_required(VERSION 3.1)

project(rton-json CXX)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS}\
		-Wall -Wextra -Werror -pedantic\
		-static\
		-s")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS}\
		-W4 -WX\
		-MT")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB SOURCES src/*.cpp include/*.hpp)

# require nlohmann_json
find_package(nlohmann_json QUIET)
if (nlohmann_json_FOUND)
	message(STATUS "FOUND nlohmann_json ${nlohmann_json_VERSION}")
else()
	add_subdirectory(third_party/json)
	message(STATUS "Using external nlohmann_json")
endif()
set(JSON_BuildTests OFF CACHE INTERNAL "")

# require fifo_map
find_package(nlohmann-fifo-map QUIET)
if (nlohmann-fifo-map_FOUND)
	message(STATUS "FOUND nlohmann-fifo-map ${nlohmann-fifo-map_VERSION}")
else()
	add_subdirectory(third_party/fifo_map)
	message(STATUS "Using external nlohmann-fifo-map")
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable(rton-json ${SOURCES})

target_link_libraries(rton-json PRIVATE nlohmann_json)
target_link_libraries(rton-json PRIVATE fifo_map)
