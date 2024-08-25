include(FetchContent)

cmake_minimum_required(VERSION 3.16)
project(audioviz LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(CMAKE_CXX_STANDARD 23)
set(AV_LIBS avfilter avformat avcodec avutil swresample swscale)

find_library(_ NAMES ${AV_LIBS} REQUIRED)
find_library(_ NAMES fftw3f REQUIRED)
find_library(_ NAMES boost_process boost_filesystem REQUIRED)

# don't build SFML's audio or network libs
set(SFML_BUILD_AUDIO FALSE CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK FALSE CACHE BOOL "" FORCE)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master)

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

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(audioviz ${SOURCES})

# reminder: always use the target_* commands instead of their global equivalents

target_compile_options(audioviz PRIVATE
	-Wno-subobject-linkage
	-Wno-narrowing)

target_include_directories(audioviz PRIVATE
	include
	${libavpp_SOURCE_DIR}/include
	${libavpp_SOURCE_DIR}/src
	${SFML_SOURCE_DIR}/include
	${spline_SOURCE_DIR}/src)

target_link_libraries(audioviz PRIVATE
	fftw3f sfml-graphics argparse boost_filesystem ${AV_LIBS})

option(AUDIOVIZ_PORTAUDIO "Build audioviz with PortAudio: allows audio playback during live rendering" ON)
option(AUDIOVIZ_LUA "Build audioviz with Lua support: allows visualizer configuration with Lua" ON)

if(AUDIOVIZ_PORTAUDIO)
	FetchContent_Declare(portaudio-pp
		GIT_REPOSITORY https://github.com/trustytrojan/portaudio-pp
		GIT_TAG main)
	FetchContent_MakeAvailable(portaudio-pp)
	target_compile_definitions(audioviz PRIVATE AUDIOVIZ_PORTAUDIO)
	target_include_directories(audioviz PRIVATE ${portaudio-pp_SOURCE_DIR}/include)
	target_link_libraries(audioviz PRIVATE portaudio)
endif()

if(AUDIOVIZ_LUA)
	FetchContent_Declare(sol2
		GIT_REPOSITORY https://github.com/ThePhD/sol2
		GIT_TAG main)
	FetchContent_MakeAvailable(sol2)
	target_compile_definitions(audioviz PRIVATE AUDIOVIZ_LUA)
	# no need for target_include_directories since sol2 is a CMake interface library
	target_link_libraries(audioviz PRIVATE lua sol2)
endif()
