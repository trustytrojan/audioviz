cmake_minimum_required(VERSION 3.18)
project(audioviz LANGUAGES CXX)

cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0167 NEW)

option(AUDIOVIZ_PORTAUDIO "Build with PortAudio: allows audio playback during live rendering" ON) # not sure where to put this yet
option(AUDIOVIZ_BOOST "Use Boost for FFmpeg process handling" OFF)
option(AUDIOVIZ_TESTS "Build the small test programs in the 'test' directory" OFF)
option(AUDIOVIZ_LUAVIZ "Build luaviz: the Lua binding" ON)
option(AUDIOVIZ_TTVIZ "Build ttviz: the first audioviz visualizer" ON)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(audioviz_compile_definitions target)
	if(LINUX)
		target_compile_definitions(${target} PUBLIC LINUX)
	endif()

	# the _WIN32 macro is defined by the mingw-w64 compilers

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_definitions(${target} PUBLIC AUDIOVIZ_DEBUG)
	endif()

	message("[audioviz_compile_definitions] added compile definitions to target '${target}'")
endfunction()

add_subdirectory(libaudioviz)

if(AUDIOVIZ_TTVIZ)
	add_subdirectory(ttviz)
endif()

if(AUDIOVIZ_TESTS)
	add_subdirectory(test)
endif()

if(AUDIOVIZ_LUAVIZ)
	add_subdirectory(luaviz)
endif()
