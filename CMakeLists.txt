include(FetchContent)

cmake_minimum_required(VERSION 3.16)
project(audioviz LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(CMAKE_CXX_STANDARD 23)
set(AV_LIBS avfilter avformat avcodec avutil swresample swscale)

# don't build SFML's audio or network libs
set(SFML_BUILD_AUDIO FALSE CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK FALSE CACHE BOOL "" FORCE)

FetchContent_Declare(
	SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
	portaudio-pp
	GIT_REPOSITORY https://github.com/trustytrojan/portaudio-pp
	GIT_TAG main
)
FetchContent_MakeAvailable(portaudio-pp)

FetchContent_Declare(
	libavpp
	GIT_REPOSITORY https://github.com/trustytrojan/libavpp
	GIT_TAG main
)
FetchContent_MakeAvailable(libavpp)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2
	GIT_TAG main
)
FetchContent_MakeAvailable(sol2)

FetchContent_Declare(
	spline
	GIT_REPOSITORY https://github.com/ttk592/spline
)
FetchContent_MakeAvailable(spline)

# make compile_commands.json for clangd!!!!!!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(audioviz ${SOURCES})

target_compile_options(audioviz PRIVATE
	-Wno-subobject-linkage
	-Wno-narrowing)

target_include_directories(audioviz PRIVATE include)
target_include_directories(audioviz PRIVATE ${argparse_SOURCE_DIR}/include)
target_include_directories(audioviz PRIVATE ${portaudio-pp_SOURCE_DIR}/include)
target_include_directories(audioviz PRIVATE ${libavpp_SOURCE_DIR}/include)
target_include_directories(audioviz PRIVATE ${libavpp_SOURCE_DIR}/src)
target_include_directories(audioviz PRIVATE ${SFML_SOURCE_DIR}/include)
target_include_directories(audioviz PRIVATE ${spline_SOURCE_DIR}/src)
target_include_directories(audioviz PRIVATE ${sol2_SOURCE_DIR}/include)

target_link_libraries(audioviz PRIVATE fftw3f lua sfml-graphics ${AV_LIBS})

# add -DNO_PORTAUDIO to cmake command to build withoit portaudio
if(NOT NO_PORTAUDIO)
	target_compile_options(audioviz PRIVATE -DPORTAUDIO)
	target_link_libraries(audioviz PRIVATE portaudio)
endif()
