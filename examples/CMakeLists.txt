link_libraries(audioviz)

function(add_example program_name)
	add_executable(${program_name} "${program_name}.cpp")
endfunction()

# add_example(stereo-scope)
add_example(ranged-spectrum)
add_example(polar-spectrum)
add_example(stereo-polar-spectrum)
add_example(shake-bass)
add_example(old-bass-nation)
add_example(spectrum-new-api)
add_example(mirrored-bass-nation)
