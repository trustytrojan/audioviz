file(GLOB AUDIOVIZ_SOURCES
	src/audioviz/fft/*.cpp
	src/audioviz/fx/*.cpp
	src/audioviz/media/*.cpp
	src/audioviz/*.cpp
)

if(LINUX AND AUDIOVIZ_LUAVIZ AND NOT GITHUB_CI)
	# relocation issues arise when luaviz links to a static libaudioviz
	add_library(audioviz SHARED ${AUDIOVIZ_SOURCES})
else()
	add_library(audioviz ${AUDIOVIZ_SOURCES})
endif()

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
