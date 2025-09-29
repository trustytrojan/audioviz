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
file(GLOB SHADER_SOURCES "${CMAKE_SOURCE_DIR}/shaders/*.frag")
set(SHADER_HEADERS)
set(SHADER_HEADER_DIR "${CMAKE_BINARY_DIR}/shader_headers")
foreach(SHADER_SOURCE IN LISTS SHADER_SOURCES)
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
    set(SHADER_HEADER "${SHADER_HEADER_DIR}/${SHADER_NAME}.h")
    list(APPEND SHADER_HEADERS ${SHADER_HEADER})
    add_custom_command(
        OUTPUT ${SHADER_HEADER}
        COMMAND ${CMAKE_COMMAND} -DSHADER_SOURCE=${SHADER_SOURCE} -DSHADER_HEADER_DIR=${SHADER_HEADER_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_shader_header.cmake
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Generating shader header ${SHADER_NAME}.h"
    )
endforeach()
add_custom_target(generate_shader_headers DEPENDS ${SHADER_HEADERS})
add_dependencies(audioviz generate_shader_headers)

target_include_directories(audioviz PRIVATE ${CMAKE_BINARY_DIR}/shader_headers)
