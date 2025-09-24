file(GLOB AUDIOVIZ_SOURCES
	src/audioviz/fft/*.cpp
	src/audioviz/fx/*.cpp
	src/audioviz/media/*.cpp
	src/audioviz/*.cpp
)

if(LINUX AND AUDIOVIZ_LUAVIZ AND NOT LUAVIZ_EXECUTABLE)
	# relocation issues arise when luaviz links to a static libaudioviz
	set(BUILD_SHARED_LIBS ON)
endif()

add_library(audioviz ${AUDIOVIZ_SOURCES})

# after the add_library so that deps.cmake can use target_XXXX commands
include(deps.cmake)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# caused by spline; safe to ignore
	target_compile_options(audioviz PUBLIC -Wno-subobject-linkage)
endif()

target_compile_options(audioviz PUBLIC -Wno-narrowing)

target_include_directories(audioviz
	PUBLIC include # our headers
	PUBLIC ${CMAKE_BINARY_DIR} # single-header libs
	PUBLIC ${portaudio-pp_SOURCE_DIR}/include
)

# popen() on MinGW expects a 'b' to differentiate text/binary,
# but on Linux it errors if a 'b' is there, so we have to do this...
if(WIN32)
	target_compile_definitions(audioviz
		PUBLIC POPEN_R_MODE="rb"
		PUBLIC POPEN_W_MODE="wb"
	)
else() # be specific if you discover more differences
	target_compile_definitions(audioviz
		PUBLIC POPEN_R_MODE="r"
		PUBLIC POPEN_W_MODE="w"
	)
endif()

# Generate header files with embedded shader source code
include(${CMAKE_SOURCE_DIR}/cmake/embed.cmake)
file(GLOB SHADER_SOURCES "${CMAKE_SOURCE_DIR}/shaders/*.frag")
foreach(shader_source IN LISTS SHADER_SOURCES)
    get_filename_component(shader_name ${shader_source} NAME)
    set(shader_header "${CMAKE_BINARY_DIR}/generated_headers/${shader_name}.h")
    set(shader_variable "audioviz_shader_${shader_name}")
    string(REPLACE "." "_" shader_variable ${shader_variable})
	embed_file_as_string_header(${shader_source} ${shader_header} ${shader_variable})
endforeach()

target_include_directories(audioviz PRIVATE ${CMAKE_BINARY_DIR}/generated_headers)
