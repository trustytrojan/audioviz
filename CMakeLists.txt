cmake_minimum_required(VERSION 3.18)
project(audioviz LANGUAGES CXX)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.24)
	cmake_policy(SET CMP0135 NEW)
endif()

option(AUDIOVIZ_USE_PORTAUDIO "Build with PortAudio: allows audio playback during live rendering" ON) # not sure where to put this yet
option(AUDIOVIZ_BUILD_EXAMPLES "Build the example programs" ON)

set(BUILD_SHARED_LIBS OFF) # static preferred
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

add_subdirectory(libavz)

if(AUDIOVIZ_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
