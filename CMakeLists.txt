include(FetchContent)

cmake_minimum_required(VERSION 3.16)
project(audioviz LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)
set(AV_LIBS avfilter avformat avcodec avutil swresample swscale)

if(WIN32)
	link_directories(
		windows_deps/ffmpeg/lib
		windows_deps/fftw
		windows_deps/portaudio
	)
	include_directories(
		windows_deps/ffmpeg/include
		windows_deps/fftw
		windows_deps/portaudio
	)
	add_compile_options(-Wa,-mbig-obj) # LuaState.cpp doesn't compile otherwise
endif()

# do it in a loop so we know exactly which lib may not be found
foreach(lib IN LISTS AV_LIBS)
	find_library(_ NAMES ${lib} REQUIRED)
endforeach()
find_library(_ NAMES fftw3f REQUIRED)

# don't build SFML's audio or network libs
set(SFML_BUILD_AUDIO FALSE CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK FALSE CACHE BOOL "" FORCE)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0-rc.1)

FetchContent_Declare(libavpp
	GIT_REPOSITORY https://github.com/trustytrojan/libavpp
	GIT_TAG main)

FetchContent_Declare(argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse)

FetchContent_Declare(spline
	GIT_REPOSITORY https://github.com/ttk592/spline)

FetchContent_MakeAvailable(SFML libavpp argparse spline)

cmake_policy(SET CMP0167 NEW)
find_package(Boost COMPONENTS process)
if(NOT Boost_FOUND)
	message("Boost not found, using FetchContent...")
	set(BOOST_INCLUDE_LIBRARIES "process")
	FetchContent_Declare(Boost
		URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.7z)
	FetchContent_MakeAvailable(Boost)
	if(WIN32)
		link_libraries(ws2_32) # winsock library required by boost.asio
	endif()
endif()

# make compile_commands.json for clangd!!!!!!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
	-Wno-subobject-linkage
	-Wno-narrowing
)

include_directories(
	include
	${libavpp_SOURCE_DIR}/include
	${libavpp_SOURCE_DIR}/src
	${SFML_SOURCE_DIR}/include
	${spline_SOURCE_DIR}/src
)

link_libraries(sfml-graphics argparse ${AV_LIBS} Boost::process)

if(WIN32)
	link_libraries(fftw3f-3)
elseif(LINUX)
	link_libraries(fftw3f)
	add_compile_definitions(LINUX)
endif()

option(AUDIOVIZ_PORTAUDIO "Build audioviz with PortAudio: allows audio playback during live rendering" ON)
option(AUDIOVIZ_LUA "Build audioviz with Lua support: allows visualizer configuration with Lua" ON)

if(AUDIOVIZ_PORTAUDIO)
	if(WIN32)
		find_library(PORTAUDIO portaudio_x64)
		if(NOT PORTAUDIO_FOUND)
			set(DLL_URL https://github.com/spatialaudio/portaudio-binaries/raw/refs/heads/master/libportaudio64bit.dll)
			file(DOWNLOAD ${DLL_URL} portaudio_x64.dll)
		endif()
		find_library(_ portaudio_x64 REQUIRED)
	else()
		find_library(_ portaudio REQUIRED)
	endif()

	FetchContent_Declare(portaudio-pp
		GIT_REPOSITORY https://github.com/trustytrojan/portaudio-pp
		GIT_TAG main)
	FetchContent_MakeAvailable(portaudio-pp)
	add_compile_definitions(AUDIOVIZ_PORTAUDIO)
	include_directories(${portaudio-pp_SOURCE_DIR}/include)

	if(WIN32)
		link_libraries(portaudio_x64)
	elseif(LINUX)
		link_libraries(portaudio)
	endif()
endif()

if(AUDIOVIZ_LUA)
	if(WIN32)
		# usual place that lua is installed via winget
		set(LUA_WINGET_PATH $ENV{localappdata}/programs/lua/lib)
		find_library(LUA lua54 PATHS ${LUA_WINGET_PATH})
	else()
		find_library(_ lua REQUIRED)
	endif()

	FetchContent_Declare(sol2
		GIT_REPOSITORY https://github.com/ThePhD/sol2
		GIT_TAG main)
	FetchContent_MakeAvailable(sol2)
	add_compile_definitions(AUDIOVIZ_LUA)

	if(WIN32)
		include_directories(windows_deps/lua/include)
		link_directories(windows_deps/lua/lib)
		link_libraries(lua54)
	elseif(LINUX)
		link_libraries(lua)
	endif()

	link_libraries(sol2)
endif()

# we need to include av/Util.cpp from libavpp for now until i figure out a better way
file(GLOB_RECURSE SOURCES src/*.cpp ${libavpp_SOURCE_DIR}/src/av/Util.cpp)
add_executable(audioviz ${SOURCES})

add_executable(scope-colors-test
	test/scope-colors-test.cpp
	src/Media.cpp
	src/tt/AudioAnalyzer.cpp
	src/tt/FrequencyAnalyzer.cpp
	src/viz/VerticalBar.cpp
	src/viz/VerticalPill.cpp
	src/tt/ColorUtils.cpp
	${libavpp_SOURCE_DIR}/src/av/Util.cpp)

add_executable(scope-test
	test/scope-test.cpp
	src/media/Media.cpp
	src/media/FfmpegCliBoostMedia.cpp
	src/tt/AudioAnalyzer.cpp
	src/tt/FrequencyAnalyzer.cpp
	src/viz/VerticalBar.cpp
	src/viz/VerticalPill.cpp
	src/tt/ColorUtils.cpp)

add_executable(spectrum-test
	test/spectrum-test.cpp
	src/viz/VerticalBar.cpp
	src/tt/FrequencyAnalyzer.cpp
	src/tt/AudioAnalyzer.cpp
	src/tt/ColorUtils.cpp
	src/media/Media.cpp
	src/media/FfmpegCliBoostMedia.cpp
	src/media/FfmpegCliPopenMedia.cpp)
