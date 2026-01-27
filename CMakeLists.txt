cmake_minimum_required(VERSION 3.18)
project(libavz LANGUAGES CXX)
include(FetchContent)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.24)
	cmake_policy(SET CMP0135 NEW)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
	# for homebrew setups
	link_directories(/opt/homebrew/lib)
	include_directories(/opt/homebrew/include)
endif()

if(CMAKE_BUILD_TYPE MATCHES Rel)
	add_compile_options(-march=native)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# some gccs don't have features in the stable libstdc++ yet
	link_libraries(stdc++exp)
endif()

option(LIBAVZ_BUILD_ANALYSIS "Build the audio analysis library" ON)
option(LIBAVZ_BUILD_GFX "Build the visualizer rendering library" ON)
option(LIBAVZ_BUILD_MEDIA "Build the media I/O library" ON)
option(LIBAVZ_BUILD_MAIN "Build the visualizer runner library" ON)
option(LIBAVZ_BUILD_EXAMPLES "Build the example programs" ON)

if(LIBAVZ_BUILD_ANALYSIS)
	add_subdirectory(src/analysis)
endif()

if(LIBAVZ_BUILD_GFX)
	add_subdirectory(src/gfx)
endif()

if(LIBAVZ_BUILD_MEDIA)
	add_subdirectory(src/media)
endif()

if(LIBAVZ_BUILD_MAIN)
	add_subdirectory(src/main)
endif()

if(LIBAVZ_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
