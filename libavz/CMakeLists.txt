include(FetchContent)

option(LIBAVZ_BUILD_ANALYSIS "Build the audio analysis library" ON)
option(LIBAVZ_BUILD_GFX "Build the graphics rendering library" ON)
option(LIBAVZ_BUILD_MEDIA "Build the media I/O library" ON)
option(LIBAVZ_BUILD_MAIN "Build the runner/player library" ON)

if(LIBAVZ_BUILD_ANALYSIS)
	add_subdirectory(analysis)
endif()

if(LIBAVZ_BUILD_GFX)
	add_subdirectory(gfx)
endif()

if(LIBAVZ_BUILD_MEDIA)
	add_subdirectory(media)
endif()

if(LIBAVZ_BUILD_MAIN)
	add_subdirectory(main)
endif()
