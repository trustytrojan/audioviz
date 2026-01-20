# Fetch argparse for command-line argument parsing
include(FetchContent)
FetchContent_Declare(
	argparse
	GIT_REPOSITORY https://github.com/p-ranav/argparse.git
	GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)

link_libraries(avz argparse)

add_library(example-framework src/ExampleFramework.cpp)
link_libraries(example-framework)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# this is also used by testing.cmake!
set(EXAMPLE_PROGRAMS
	stereo-scope
	ranged-spectrum
	polar-spectrum
	stereo-polar-spectrum
	shake-bass
	old-bass-nation
	mirrored-bass-nation
	scope
	log-spectrum
	particle-system
	polar-particle-system
)

foreach(example ${EXAMPLE_PROGRAMS})
	add_executable(${example} "src/${example}.cpp")
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
