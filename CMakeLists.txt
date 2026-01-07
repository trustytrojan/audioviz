cmake_minimum_required(VERSION 3.18)
project(audioviz LANGUAGES CXX)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.24)
	cmake_policy(SET CMP0135 NEW)
endif()

option(AUDIOVIZ_USE_PORTAUDIO "Build with PortAudio: allows audio playback during live rendering" ON) # not sure where to put this yet
option(AUDIOVIZ_BUILD_EXAMPLES "Build the example programs" ON)
option(AUDIOVIZ_BUILD_LUAVIZ "Build the Lua binding" ON)
option(AUDIOVIZ_BUILD_TTVIZ "Build the reference audioviz implementation" ON)

set(BUILD_SHARED_LIBS OFF) # static preferred
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
	# for homebrew setups
	link_directories(/opt/homebrew/lib)
	include_directories(/opt/homebrew/include)
endif()

add_subdirectory(libaudioviz)

if(AUDIOVIZ_BUILD_TTVIZ)
	add_subdirectory(ttviz)
endif()

if(AUDIOVIZ_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(AUDIOVIZ_BUILD_LUAVIZ)
	add_subdirectory(luaviz)
endif()
