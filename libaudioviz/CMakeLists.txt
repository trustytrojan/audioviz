option(LIBAUDIOVIZ_BOOST "Use Boost::process over popen() for spawning ffmpeg" OFF)

file(GLOB AUDIOVIZ_SOURCES
	src/audioviz/fft/*.cpp
	src/audioviz/fx/*.cpp
	src/audioviz/media/FfmpegPopenEncoder.cpp
	src/audioviz/media/FfmpegPopenMedia.cpp
	src/audioviz/media/Media.cpp
	src/audioviz/*.cpp
)

if(AUDIOVIZ_BOOST)
	list(APPEND AUDIOVIZ_SOURCES
		src/audioviz/media/FfmpegBoostEncoder.cpp
		src/audioviz/media/FfmpegBoostMedia.cpp
	)
endif()

if(LINUX)
	# relocation issues arise when linking luaviz to a static libaudioviz
	# just avoid it on linux by making it shared
	add_library(audioviz SHARED ${AUDIOVIZ_SOURCES})
else()
	add_library(audioviz STATIC ${AUDIOVIZ_SOURCES})
endif()
audioviz_compile_definitions(audioviz)

# after the add_library so that deps.cmake can use target_XXXX commands
include(deps.cmake)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# caused by spline; safe to ignore
	target_compile_options(audioviz PUBLIC -Wno-subobject-linkage)
endif()

target_compile_options(audioviz PUBLIC -Wno-narrowing)

target_include_directories(audioviz
	PUBLIC include # our headers
	PUBLIC ${CMAKE_BINARY_DIR} # individually-downloaded headers
	PUBLIC ${portaudio-pp_SOURCE_DIR}/include
)
