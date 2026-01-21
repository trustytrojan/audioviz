# deps.cmake - fetch, setup, and link to dependencies
include(FetchContent)

### TEMPORARY - libavz should not be responsible for audio playback.
### but to keep things stable i will leave this as is for now.
## portaudio (optional)
if(LIBAVZ_USE_PORTAUDIO)
	FetchContent_Declare(portaudio-pp URL https://github.com/trustytrojan/portaudio-pp/archive/main.tar.gz)
	FetchContent_MakeAvailable(portaudio-pp)
	target_compile_definitions(avz PUBLIC LIBAVZ_PORTAUDIO)
	target_link_libraries(avz PUBLIC portaudio-pp::portaudio-pp)
endif()
