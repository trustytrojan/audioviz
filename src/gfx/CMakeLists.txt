file(GLOB LIBAVZ_GFX_SOURCES *.cpp fx/*.cpp)
add_library(avz-gfx ${LIBAVZ_GFX_SOURCES})
add_library(avz::gfx ALIAS avz-gfx)

target_compile_options(avz-gfx PUBLIC -Wno-narrowing -ffast-math)
target_include_directories(avz-gfx PUBLIC ${CMAKE_SOURCE_DIR}/include)

# sfml
if(WIN32)
	# reduce dynamic linker hassle
	set(SFML_STATIC_LIBRARIES ON CACHE BOOL "")
endif()
find_package(SFML COMPONENTS Graphics Window System QUIET)
if(NOT SFML_FOUND)
	message(STATUS "[avz-gfx] sfml: FIND_SFML_ERROR: ${FIND_SFML_ERROR}")
	if(WIN32)
		message(STATUS "[avz-gfx] sfml: windows detected: fetching binaries")
		FetchContent_Declare(sfml URL https://github.com/SFML/SFML/releases/download/3.0.2/SFML-3.0.2-Windows.MinGW.x64.zip)
		FetchContent_MakeAvailable(sfml)
		list(APPEND CMAKE_PREFIX_PATH ${sfml_SOURCE_DIR}/lib/cmake/SFML)
	else()
		message(STATUS "[avz-gfx] sfml: fetching source")
		set(SFML_BUILD_AUDIO OFF)
		set(SFML_BUILD_NETWORK OFF)
		FetchContent_Declare(sfml URL https://github.com/SFML/SFML/archive/3.0.2.tar.gz)
		FetchContent_MakeAvailable(sfml)
	endif()
endif()
find_package(SFML COMPONENTS Graphics Window System REQUIRED)
target_link_libraries(avz-gfx PUBLIC SFML::Graphics)

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
		COMMENT "[avz-gfx] Generating shader header ${SHADER_NAME}.h"
	)
endforeach()
add_custom_target(generate_shader_headers DEPENDS ${SHADER_HEADERS})
add_dependencies(avz-gfx generate_shader_headers)

target_include_directories(avz-gfx PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/)
