file(GLOB_RECURSE LIBAVZ_MAIN_SOURCES src/*.cpp)
add_library(avz-main ${LIBAVZ_MAIN_SOURCES})
add_library(avz::main ALIAS avz-main)

target_compile_options(avz-main PUBLIC -Wno-narrowing -ffast-math)
target_include_directories(avz-main PUBLIC include)
target_link_libraries(avz-main PUBLIC avz::gfx avz::media)

# TEMPORARY - libavz should not be responsible for audio playback.
# but to keep things stable i will leave this as is for now.
# portaudio (optional)
if(LIBAVZ_USE_PORTAUDIO)
	FetchContent_Declare(portaudio-pp URL https://github.com/trustytrojan/portaudio-pp/archive/main.tar.gz)
	FetchContent_MakeAvailable(portaudio-pp)
	target_compile_definitions(avz-main PUBLIC LIBAVZ_PORTAUDIO)
	target_link_libraries(avz-main PUBLIC portaudio-pp::portaudio-pp)
endif()
