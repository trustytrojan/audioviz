## project setup
include(FetchContent)
cmake_minimum_required(VERSION 3.18)
project(audioviz LANGUAGES CXX)

cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0167 NEW)

option(AUDIOVIZ_PORTAUDIO "Build audioviz with PortAudio: allows audio playback during live rendering" ON) # not sure where to put this yet
option(AUDIOVIZ_TESTS "Build the small test programs in the 'test' directory" OFF)

set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(LINUX)
	add_compile_definitions(LINUX)
endif()

add_compile_options(
	-Wno-subobject-linkage # caused by spline; safe to ignore
	-Wno-narrowing
)

add_subdirectory(libaudioviz)
add_subdirectory(ttviz)
if(AUDIOVIZ_TESTS)
	add_subdirectory(test)
endif()
