include(FetchContent)

cmake_minimum_required(VERSION 3.16)
project(audioviz LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_CXX_STANDARD 23)
set(AV_LIBS avfilter avformat avcodec avutil swresample swscale)

if(WIN32)
	# check README to understand this
	set(CMAKE_PREFIX_PATH
		windows_deps/ffmpeg/lib)
	link_directories(
		windows_deps/ffmpeg/lib
		windows_deps/fftw
		windows_deps/portaudio)
	include_directories(
		windows_deps/ffmpeg/include
		windows_deps/fftw
		windows_deps/portaudio
		windows_deps)
endif()

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
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git)

FetchContent_Declare(spline
	GIT_REPOSITORY https://github.com/ttk592/spline)

FetchContent_MakeAvailable(SFML libavpp argparse spline)

# make compile_commands.json for clangd!!!!!!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
	-Wno-subobject-linkage
	-Wno-narrowing)

include_directories(
	include
	${libavpp_SOURCE_DIR}/include
	${libavpp_SOURCE_DIR}/src
	${SFML_SOURCE_DIR}/include
	${spline_SOURCE_DIR}/src)

link_libraries(sfml-graphics argparse ${AV_LIBS})

if(WIN32)
	link_libraries(fftw3f-3)
elseif(LINUX)
	link_libraries(fftw3f)
endif()

option(AUDIOVIZ_PORTAUDIO "Build audioviz with PortAudio: allows audio playback during live rendering" ON)
option(AUDIOVIZ_LUA "Build audioviz with Lua support: allows visualizer configuration with Lua" OFF)

if(AUDIOVIZ_PORTAUDIO)
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
	FetchContent_Declare(sol2
		GIT_REPOSITORY https://github.com/ThePhD/sol2
		GIT_TAG main)
	FetchContent_MakeAvailable(sol2)
	add_compile_definitions(AUDIOVIZ_LUA)
	# no need for target_include_directories since sol2 is a CMake interface library
	link_libraries(lua sol2)
endif()

# we need to include av/Util.cpp from libavpp for now until i figure out a better way
file(GLOB_RECURSE SOURCES src/*.cpp ${libavpp_SOURCE_DIR}/src/av/Util.cpp)
add_executable(audioviz ${SOURCES})

# add_executable(scope-drawable-test
# 	ScopeDrawableTest.cpp
# 	src/Media.cpp
# 	src/tt/AudioAnalyzer.cpp
# 	src/tt/FrequencyAnalyzer.cpp
# 	src/viz/VerticalBar.cpp
# 	src/viz/VerticalPill.cpp
# 	src/tt/ColorUtils.cpp
# 	${libavpp_SOURCE_DIR}/src/av/Util.cpp)

# add_executable(rotate-test rotate-test.cpp)
