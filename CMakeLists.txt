cmake_minimum_required(VERSION 3.18)
project(audioviz LANGUAGES CXX)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.24)
	cmake_policy(SET CMP0135 NEW)
endif()

option(AUDIOVIZ_USE_PORTAUDIO "Build with PortAudio: allows audio playback during live rendering" ON) # not sure where to put this yet
option(AUDIOVIZ_TESTS "Build the small test programs in the 'test' directory" OFF)
option(AUDIOVIZ_LUAVIZ "Build luaviz: the Lua binding" ON)
option(AUDIOVIZ_TTVIZ "Build ttviz: the first audioviz visualizer" ON)

set(BUILD_SHARED_LIBS OFF) # static preferred
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# define the path of our additional CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(APPLE)
	# for homebrew setups
	link_directories(/opt/homebrew/lib)
	include_directories(/opt/homebrew/include)
endif()

add_subdirectory(libaudioviz)

if(AUDIOVIZ_TTVIZ)
	add_subdirectory(ttviz)
endif()

if(AUDIOVIZ_TESTS)
	add_subdirectory(tests)
endif()

if(AUDIOVIZ_LUAVIZ)
	add_subdirectory(luaviz)
endif()
