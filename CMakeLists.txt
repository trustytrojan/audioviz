## project setup
include(FetchContent)
cmake_minimum_required(VERSION 3.16)
project(audioviz LANGUAGES CXX)

cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0167 NEW)

option(AUDIOVIZ_PORTAUDIO "Build audioviz with PortAudio: allows audio playback during live rendering" ON)
option(AUDIOVIZ_LUA "Build audioviz with Lua support: allows visualizer configuration with Lua" ON)
option(AUDIOVIZ_TESTS "Build the small test programs in the 'test' directory" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)
set(FFMPEG_LIBS avformat avutil avcodec)

## misc OS-specific things
if(WIN32)
	add_compile_options(-Wa,-mbig-obj) # LuaState.cpp doesn't compile otherwise
elseif(LINUX)
	add_compile_definitions(LINUX)
endif()

## ffmpeg
find_library(FFMPEG_LIBS_FOUND NAMES ${FFMPEG_LIBS})
if(WIN32 AND NOT FFMPEG_LIBS_FOUND)
	find_program(WINGET winget)
	if(NOT WINGET)
		# we'll download ffmpeg ourselves
		message("winget not found, downloading latest ffmpeg with FetchContent_Declare")
		FetchContent_Declare(ffmpeg URL https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z)
		FetchContent_MakeAvailable(ffmpeg)
		list(APPEND CMAKE_PREFIX_PATH ${ffmpeg_SOURCE_DIR}/lib)
		link_directories(${ffmpeg_SOURCE_DIR}/lib)
		include_directories(${ffmpeg_SOURCE_DIR}/include)
	else()
		# use winget installation of ffmpeg shared libs
		file(GLOB FFMPEG_WINGET_PATH $ENV{LocalAppData}/Microsoft/WinGet/Packages/Gyan.FFmpeg.Shared_Microsoft.Winget.Source_8wekyb3d8bbwe/ffmpeg-*-full_build-shared)
		message("winget found, attempting to use ffmpeg installation path: ${FFMPEG_WINGET_PATH}")
		list(APPEND CMAKE_PREFIX_PATH ${FFMPEG_WINGET_PATH}/lib)
		link_directories(${FFMPEG_WINGET_PATH}/lib)
		include_directories(${FFMPEG_WINGET_PATH}/include)
	endif()
endif()

# find each lib separately as the NAMES parameter only checks ONE of the names
foreach(LIB IN LISTS FFMPEG_LIBS)
	find_library(${LIB} NAMES ${LIB} REQUIRED)
endforeach()

## fftw
if(WIN32)
	FetchContent_Declare(fftw URL https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip)
	FetchContent_MakeAvailable(fftw)
	link_directories(${fftw_SOURCE_DIR})
	include_directories(${fftw_SOURCE_DIR})
	link_libraries(fftw3f-3)
elseif(LINUX)
	link_libraries(fftw3f)
endif()

## sfml (without audio and network libs)
set(SFML_BUILD_AUDIO FALSE CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK FALSE CACHE BOOL "" FORCE)
FetchContent_Declare(
	SFML
    GIT_REPOSITORY https://github.com/SFML/SFML
    GIT_TAG 3.0.0
)

## boost
find_package(Boost COMPONENTS process)
if(NOT Boost_FOUND)
	message("Boost not found, using FetchContent...")
	set(BOOST_INCLUDE_LIBRARIES "process")
	FetchContent_Declare(Boost URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.7z)
	FetchContent_MakeAvailable(Boost)
	if(WIN32)
		link_libraries(ws2_32) # winsock library required by boost.asio
	endif()
endif()

## header-only libs
FetchContent_Declare(
	libavpp
	GIT_REPOSITORY https://github.com/trustytrojan/libavpp
	GIT_TAG main
)

FetchContent_Declare(
	argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse
)

FetchContent_Declare(
	spline
	GIT_REPOSITORY https://github.com/ttk592/spline
)

FetchContent_MakeAvailable(SFML libavpp argparse spline)

add_compile_options(
	-Wno-subobject-linkage # caused by spline; safe to ignore
	-Wno-narrowing
)

include_directories(
	include
	${libavpp_SOURCE_DIR}/include
	${libavpp_SOURCE_DIR}/src
	${SFML_SOURCE_DIR}/include
	${spline_SOURCE_DIR}/src
)

link_libraries(sfml-graphics argparse ${FFMPEG_LIBS} Boost::process)

## portaudio (optional)
if(AUDIOVIZ_PORTAUDIO)
	if(WIN32)
		file(DOWNLOAD https://github.com/spatialaudio/portaudio-binaries/raw/refs/heads/master/libportaudio64bit.dll portaudio_x64.dll)
		file(DOWNLOAD https://github.com/PortAudio/portaudio/raw/refs/tags/v19.7.0/include/portaudio.h portaudio.h)
		include_directories(${CMAKE_BINARY_DIR})
		link_directories(${CMAKE_BINARY_DIR})
	endif()

	FetchContent_Declare(
		portaudio-pp
		GIT_REPOSITORY https://github.com/trustytrojan/portaudio-pp
		GIT_TAG main
	)
	FetchContent_MakeAvailable(portaudio-pp)
	add_compile_definitions(AUDIOVIZ_PORTAUDIO)
	include_directories(${portaudio-pp_SOURCE_DIR}/include)

	if(WIN32)
		link_libraries(portaudio_x64)
	elseif(LINUX)
		link_libraries(portaudio)
	endif()
endif()

## lua (optional)
if(AUDIOVIZ_LUA)
	if(WIN32)
		# usual place that lua is installed via winget
		set(WINGET_LUA_PATH $ENV{localappdata}/programs/lua)
		list(APPEND CMAKE_PREFIX_PATH ${WINGET_LUA_PATH}/lib)
	endif()
	find_library(lua NAMES lua lua54 REQUIRED)

	FetchContent_Declare(
		sol2
		GIT_REPOSITORY https://github.com/ThePhD/sol2
		GIT_TAG main
	)
	FetchContent_MakeAvailable(sol2)
	add_compile_definitions(AUDIOVIZ_LUA)

	if(WIN32)
		include_directories(${WINGET_LUA_PATH}/include)
		link_directories(${WINGET_LUA_PATH}/lib)
		link_libraries(lua54)
	elseif(LINUX)
		link_libraries(lua)
	endif()

	link_libraries(sol2)
endif()

# do this now instead of earlier so we only make compile_commands.json for our code
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE AUDIOVIZ_SOURCES src/*.cpp)
add_executable(audioviz ${AUDIOVIZ_SOURCES})

if(AUDIOVIZ_TESTS)
	add_executable(scope-colors-test
		test/scope-colors-test.cpp
		src/media/Media.cpp
		src/media/FfmpegCliBoostMedia.cpp
		src/tt/AudioAnalyzer.cpp
		src/tt/FrequencyAnalyzer.cpp
		src/viz/VerticalBar.cpp
		src/viz/VerticalPill.cpp
		src/tt/ColorUtils.cpp
	)
	add_executable(scope-test
		test/scope-test.cpp
		src/media/Media.cpp
		src/media/FfmpegCliBoostMedia.cpp
		src/tt/AudioAnalyzer.cpp
		src/tt/FrequencyAnalyzer.cpp
		src/viz/VerticalBar.cpp
		src/viz/VerticalPill.cpp
		src/tt/ColorUtils.cpp
	)
	add_executable(spectrum-test
		test/spectrum-test.cpp
		src/viz/VerticalBar.cpp
		src/tt/FrequencyAnalyzer.cpp
		src/tt/AudioAnalyzer.cpp
		src/tt/ColorUtils.cpp
		src/media/Media.cpp
		src/media/FfmpegCliBoostMedia.cpp
	)
endif()
