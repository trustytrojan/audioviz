file(GLOB AUDIOVIZ_SOURCES
	src/audioviz/fft/*.cpp
	src/audioviz/fx/*.cpp
	src/audioviz/media/*.cpp
	src/audioviz/*.cpp
)

add_library(audioviz ${AUDIOVIZ_SOURCES})

# after the add_library so that deps.cmake can use target_XXXX commands
include(deps.cmake)

target_include_directories(audioviz
	PUBLIC include # our headers
	PUBLIC ${CMAKE_BINARY_DIR} # individually-downloaded headers
	PUBLIC ${portaudio-pp_SOURCE_DIR}/include
)
