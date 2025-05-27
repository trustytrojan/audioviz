file(GLOB AUDIOVIZ_SOURCES
	src/audioviz/fft/*.cpp
	src/audioviz/fx/*.cpp
	src/audioviz/media/*.cpp
	src/audioviz/*.cpp
)

if(LINUX AND AUDIOVIZ_LUAVIZ)
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
