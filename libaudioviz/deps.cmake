# deps.cmake - This file only fetches dependencies and makes them available to the calling project/subdirectory.
include(FetchContent)

## ffmpeg
set(FFMPEG_LIBS avformat avutil avcodec)
find_library(FFMPEG_LIBS_FOUND NAMES ${FFMPEG_LIBS})
if(WIN32 AND NOT FFMPEG_LIBS_FOUND)
	find_program(WINGET winget)
	if(NOT WINGET)
		# we'll download ffmpeg ourselves
		message("winget not found, downloading latest ffmpeg with FetchContent_Declare")
		FetchContent_Declare(ffmpeg URL https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z)
		FetchContent_MakeAvailable(ffmpeg)
		list(APPEND CMAKE_PREFIX_PATH ${ffmpeg_SOURCE_DIR}/lib)
		link_directories(${ffmpeg_SOURCE_DIR}/lib)
		include_directories(${ffmpeg_SOURCE_DIR}/include)
	else()
		# use winget installation of ffmpeg shared libs
		file(GLOB FFMPEG_WINGET_PATH $ENV{LocalAppData}/Microsoft/WinGet/Packages/Gyan.FFmpeg.Shared_Microsoft.Winget.Source_8wekyb3d8bbwe/ffmpeg-*-full_build-shared)
		message("winget found, attempting to use ffmpeg installation path: ${FFMPEG_WINGET_PATH}")
		list(APPEND CMAKE_PREFIX_PATH ${FFMPEG_WINGET_PATH}/lib)
		link_directories(${FFMPEG_WINGET_PATH}/lib)
		include_directories(${FFMPEG_WINGET_PATH}/include)
	endif()
endif()

# find each lib separately as the NAMES parameter only checks for one of the names being present
foreach(LIB IN LISTS FFMPEG_LIBS)
	find_library(${LIB} NAMES ${LIB} REQUIRED)
endforeach()

## fftw
if(WIN32)
	FetchContent_Declare(fftw URL https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip)
	FetchContent_MakeAvailable(fftw)
	link_directories(${fftw_SOURCE_DIR})
	include_directories(${fftw_SOURCE_DIR})
	link_libraries(fftw3f-3)
elseif(LINUX)
	link_libraries(fftw3f)
endif()

## sfml (without audio and network libs)
set(SFML_BUILD_AUDIO FALSE CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK FALSE CACHE BOOL "" FORCE)
FetchContent_Declare(
	SFML
	URL https://github.com/SFML/SFML/archive/3.0.0.zip
)

## boost
find_package(Boost COMPONENTS process)
if(NOT Boost_FOUND)
	message("Boost not found, using FetchContent...")
	set(BOOST_INCLUDE_LIBRARIES "process")
	FetchContent_Declare(Boost URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.7z)
	FetchContent_MakeAvailable(Boost)
	if(WIN32)
		link_libraries(ws2_32) # winsock library required by boost.asio
	endif()
endif()

## header-only libs
FetchContent_Declare(
	libavpp
	URL https://github.com/trustytrojan/libavpp/archive/main.zip
)
file(DOWNLOAD https://github.com/p-ranav/argparse/raw/master/include/argparse/argparse.hpp ${CMAKE_BINARY_DIR}/argparse.hpp)
file(DOWNLOAD https://github.com/ttk592/spline/raw/master/src/spline.h ${CMAKE_BINARY_DIR}/spline.h)

FetchContent_MakeAvailable(SFML libavpp)

### TEMPORARY - libaudioviz should not be responsible for audio playback.
### but to keep things stable i will leave this as is for now.
## portaudio (optional)
if(AUDIOVIZ_PORTAUDIO)
	if(WIN32)
		file(DOWNLOAD https://github.com/spatialaudio/portaudio-binaries/raw/refs/heads/master/libportaudio64bit.dll ${CMAKE_BINARY_DIR}/portaudio_x64.dll)
		file(DOWNLOAD https://github.com/PortAudio/portaudio/raw/refs/tags/v19.7.0/include/portaudio.h ${CMAKE_BINARY_DIR}/portaudio.h)
		link_directories(${CMAKE_BINARY_DIR})
	endif()

	FetchContent_Declare(
		portaudio-pp
		URL https://github.com/trustytrojan/portaudio-pp/archive/main.zip
	)
	FetchContent_MakeAvailable(portaudio-pp)
	add_compile_definitions(AUDIOVIZ_PORTAUDIO)
	include_directories(${portaudio-pp_SOURCE_DIR}/include)

	if(WIN32)
		link_libraries(portaudio_x64)
	elseif(LINUX)
		link_libraries(portaudio)
	endif()
endif()

## lua (optional)
if(AUDIOVIZ_LUA)
	if(WIN32)
		# usual place that lua is installed via winget
		set(WINGET_LUA_PATH $ENV{localappdata}/programs/lua)
		list(APPEND CMAKE_PREFIX_PATH ${WINGET_LUA_PATH}/lib)
	endif()
	find_library(lua NAMES lua lua54 REQUIRED)

	file(DOWNLOAD https://github.com/ThePhD/sol2/releases/download/v3.3.0/sol.hpp ${CMAKE_BINARY_DIR}/sol/sol.hpp)
	file(DOWNLOAD https://github.com/ThePhD/sol2/releases/download/v3.3.0/config.hpp ${CMAKE_BINARY_DIR}/sol/config.hpp)
	add_compile_definitions(AUDIOVIZ_LUA)

	if(WIN32)
		include_directories(${WINGET_LUA_PATH}/include)
		link_directories(${WINGET_LUA_PATH}/lib)
		link_libraries(lua54)
	elseif(LINUX)
		link_libraries(lua)
	endif()
endif()
