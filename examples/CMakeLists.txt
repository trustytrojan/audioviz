# Fetch argparse for command-line argument parsing
include(FetchContent)
FetchContent_Declare(
	argparse
	GIT_REPOSITORY https://github.com/p-ranav/argparse.git
	GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)

link_libraries(audioviz argparse)

function(add_example program_name)
	add_executable(${program_name} "${program_name}.cpp")
endfunction()

add_example(stereo-scope)
add_example(ranged-spectrum)
add_example(polar-spectrum)
add_example(stereo-polar-spectrum)
add_example(shake-bass)
add_example(old-bass-nation)
add_example(spectrum-new-api)
add_example(mirrored-bass-nation)
add_example(scope)

include(testing.cmake)
