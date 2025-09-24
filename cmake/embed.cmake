function(embed_file_as_string_header input_file output_header variable_name)
    file(READ ${input_file} file_content)

    # Create a C++ raw string literal
    set(header_content "#include <string_view>\ninline const std::string_view ${variable_name}{R\"glsl(${file_content})glsl\"};")

    # Write the header file
    file(WRITE ${output_header} "${header_content}")
endfunction()
