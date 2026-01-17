file(GLOB AUDIOVIZ_SOURCES
	src/audioviz/fft/*.cpp
	src/audioviz/fx/*.cpp
	src/audioviz/media/*.cpp
	src/audioviz/*.cpp
)

add_library(audioviz ${AUDIOVIZ_SOURCES})
include(deps.cmake)

target_compile_options(audioviz PUBLIC -Wno-narrowing -ffast-math)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# caused by tk-spline, safe to ignore
	target_compile_options(audioviz PUBLIC -Wno-subobject-linkage)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	# caused by nlohmann/json, safe to ignore
	target_compile_options(audioviz PUBLIC -Wno-nan-infinity-disabled)
endif()

target_include_directories(audioviz PUBLIC
	include # our public headers

	# spline & json need to be exposed since our public headers include them.
	# if at a later time we decide that encapsulation is important, this must be changed.
	${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(audioviz PRIVATE
	${portaudio-pp_SOURCE_DIR}/include # for Base::start_in_window
)

# popen() on MinGW expects a 'b' to differentiate text/binary,
# but on Linux it errors if a 'b' is there, so we have to do this...
if(WIN32)
	target_compile_definitions(audioviz PRIVATE POPEN_R_MODE="rb" POPEN_W_MODE="wb")
else()
	target_compile_definitions(audioviz PRIVATE POPEN_R_MODE="r" POPEN_W_MODE="w")
endif()

# Generate header files with embedded shader source code
file(GLOB SHADER_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
	"${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
)
set(SHADER_HEADERS)
set(SHADER_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/shader_headers")
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

target_include_directories(audioviz PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/shader_headers)
