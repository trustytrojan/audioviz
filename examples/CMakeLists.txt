# Fetch argparse for command-line argument parsing
include(FetchContent)
FetchContent_Declare(
	argparse
	GIT_REPOSITORY https://github.com/p-ranav/argparse.git
	GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)

link_libraries(audioviz argparse)

set(EXAMPLE_PROGRAMS
	stereo-scope
	ranged-spectrum
	polar-spectrum
	stereo-polar-spectrum
	shake-bass
	old-bass-nation
	spectrum-new-api
	mirrored-bass-nation
	scope
)

foreach(example ${EXAMPLE_PROGRAMS})
	add_executable(${example} "${example}.cpp")
endforeach()

include(testing.cmake)
