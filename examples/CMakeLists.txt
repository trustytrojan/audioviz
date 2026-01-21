# Fetch argparse for command-line argument parsing
include(FetchContent)
FetchContent_Declare(
	argparse
	GIT_REPOSITORY https://github.com/p-ranav/argparse.git
	GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)

link_libraries(avz argparse)

add_library(example-framework
	src/framework/ExampleFramework.cpp
	src/framework/BassNationSpectrumLayer.cpp
)
link_libraries(example-framework)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# this is also used by testing.cmake!
file(GLOB EXAMPLE_SOURCES "src/*.cpp")
set(EXAMPLE_PROGRAMS "")
foreach(source ${EXAMPLE_SOURCES})
	get_filename_component(example ${source} NAME_WE)
	list(APPEND EXAMPLE_PROGRAMS ${example})
	add_executable(${example} ${source})
endforeach()

if(WIN32)
	# fftw & portaudio are DLLs, copy them to our binary dir so that we don't have to modify PATH
	file(COPY_FILE
		${CMAKE_BINARY_DIR}/_deps/fftw-src/libfftw3f-3.dll
		${CMAKE_CURRENT_BINARY_DIR}/libfftw3f-3.dll
		ONLY_IF_DIFFERENT)
	if(LIBAVZ_USE_PORTAUDIO)
		file(COPY_FILE
			${portaudio-pp_BINARY_DIR}/portaudio_x64.dll
			${CMAKE_CURRENT_BINARY_DIR}/portaudio_x64.dll
			ONLY_IF_DIFFERENT)
	endif()
endif()

include(testing.cmake)
