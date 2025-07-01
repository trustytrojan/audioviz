# deps.cmake - fetch, setup, and link to dependencies
include(FetchContent)

## GLEW (caused by pbo optimization changes)
if(NOT WIN32)
	find_package(GLEW REQUIRED)
	target_link_libraries(audioviz PUBLIC GLEW::GLEW)
endif()

## OpenGL (separate on macos????? also caused by optimization changes)
if(APPLE)
	find_package(OpenGL COMPONENTS OpenGL REQUIRED)
	target_link_libraries(audioviz PUBLIC OpenGL::GL)
endif()

## ffmpeg (just the ffmpeg & ffprobe executables, libs no longer needed)
if(NOT GITHUB_CI)
	find_program(FFMPEG NAMES ffmpeg ffprobe)
	if(WIN32 AND FFMPEG STREQUAL "FFMPEG-NOTFOUND")
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
			# this needs to be rewritten, we no longer need to link to ffmpeg libs
			find_program(WINGET winget)
			if(WINGET STREQUAL "WINGET-NOTFOUND")
				# we'll download ffmpeg ourselves (rare case, as windows usually comes with winget nowadays)
				message("winget not found, downloading latest ffmpeg with FetchContent_Declare")
				FetchContent_Declare(ffmpeg URL https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.7z)
				FetchContent_MakeAvailable(ffmpeg)
				list(APPEND CMAKE_PROGRAM_PATH ${ffmpeg_SOURCE_DIR}/bin)
			else()
				# use winget installation of ffmpeg shared libs
				message("winget found!")
				file(GLOB FFMPEG_WINGET_PATH $ENV{LocalAppData}/Microsoft/WinGet/Packages/Gyan.FFmpeg_Microsoft.Winget.Source_8wekyb3d8bbwe/ffmpeg-*-full_build)
				if(NOT FFMPEG_WINGET_PATH)
					message(FATAL_ERROR "ffmpeg not installed! please install it first by running 'winget install ffmpeg'")
				endif()
				message("using ffmpeg path: ${FFMPEG_WINGET_PATH}")
				list(APPEND CMAKE_PROGRAM_PATH ${FFMPEG_WINGET_PATH}/bin)
			endif()
		elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
			message("windows arm64 detected, downloading ffmpeg from https://github.com/tordona/ffmpeg-win-arm64")

			# unfortunately cmake itself can't handle all 7zips properly
			find_program(7zip 7z)
			if(7zip STREQUAL "7zip-NOTFOUND")
				message(FATAL_ERROR "a 7zip-like program is required to extract ffmpeg for windows arm64!")
			endif()

			set(FFMPEG_PATH "${CMAKE_BINARY_DIR}/_deps/ffmpeg")
			file(DOWNLOAD https://github.com/tordona/ffmpeg-win-arm64/releases/download/7.1.1/ffmpeg-7.1.1-essentials-shared-win-arm64.7z "${FFMPEG_PATH}/ffmpeg.7z")

			if(NOT IS_DIRECTORY "${FFMPEG_PATH}/bin")
				message("extracting ffmpeg 7zip...")
				execute_process(
					COMMAND 7z x -y "-o${FFMPEG_PATH}" "${FFMPEG_PATH}/ffmpeg.7z"
					COMMAND_ERROR_IS_FATAL ANY
				)
			endif()

			list(APPEND CMAKE_PROGRAM_PATH ${FFMPEG_PATH}/bin)
		endif()
	endif()
	find_program(FFMPEG ffmpeg REQUIRED)
	find_program(FFPROBE ffprobe REQUIRED)
endif()

## fftw
if(NOT ANDROID)
	# termux's fftw package is missing FFTW3LibraryDepends.cmake, so don't bother finding
	find_package(FFTW3 COMPONENTS fftw3f QUIET)
endif()
if(WIN32 AND NOT FFTW3_FOUND)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		message("fetching fftw x64 binaries...")
		FetchContent_Declare(fftw URL https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip)
		FetchContent_MakeAvailable(fftw)
		target_link_directories(audioviz PUBLIC ${fftw_SOURCE_DIR})
		target_include_directories(audioviz PUBLIC ${fftw_SOURCE_DIR})
		target_link_libraries(audioviz PUBLIC fftw3f-3)
	else()
		message("other architecture detected, fetching fftw source...")
		set(BUILD_TESTS OFF)
		set(ENABLE_FLOAT ON)
		set(DISABLE_FORTRAN ON)
		FetchContent_Declare(fftw URL https://www.fftw.org/fftw-3.3.10.tar.gz)
		FetchContent_MakeAvailable(fftw)
		target_include_directories(audioviz PUBLIC ${fftw_SOURCE_DIR}/api)
		target_link_libraries(audioviz PUBLIC fftw3f)
	endif()
else()
	target_link_libraries(audioviz PUBLIC fftw3f)
endif()

## sfml (without audio and network libs)
find_package(SFML COMPONENTS Graphics Window System QUIET)
if(NOT SFML_FOUND)
	message(STATUS "SFML not found, using FetchContent...")
    set(SFML_BUILD_AUDIO OFF)
    set(SFML_BUILD_NETWORK OFF)
	FetchContent_Declare(SFML URL https://github.com/SFML/SFML/archive/3.0.1.zip)
	FetchContent_MakeAvailable(SFML)
endif()
target_link_libraries(audioviz PUBLIC SFML::Graphics)

## spline
file(DOWNLOAD https://github.com/ttk592/spline/raw/master/src/spline.h ${CMAKE_BINARY_DIR}/tk-spline.hpp)

## nlohmann_json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(audioviz PUBLIC nlohmann_json::nlohmann_json)

### TEMPORARY - libaudioviz should not be responsible for audio playback.
### but to keep things stable i will leave this as is for now.
## portaudio (optional)
if(AUDIOVIZ_USE_PORTAUDIO)
	if(WIN32)
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
			message("windows x64 detected, fetching portaudio dll...")
			file(DOWNLOAD https://github.com/spatialaudio/portaudio-binaries/raw/refs/heads/master/libportaudio64bit.dll ${CMAKE_BINARY_DIR}/portaudio_x64.dll)
			file(DOWNLOAD https://github.com/PortAudio/portaudio/raw/refs/tags/v19.7.0/include/portaudio.h ${CMAKE_BINARY_DIR}/portaudio.h)
			target_link_directories(audioviz PUBLIC ${CMAKE_BINARY_DIR})
			target_link_libraries(audioviz PUBLIC portaudio_x64)
		else()
			message("other architecture detected, fetching portaudio source...")
			# this is a snapshot of their git master btw
			FetchContent_Declare(portaudio URL https://files.portaudio.com/archives/pa_snapshot.tgz)
			FetchContent_MakeAvailable(portaudio)
			target_link_libraries(audioviz PUBLIC portaudio)
		endif()
	else()
		find_library(portaudio portaudio REQUIRED)
		target_link_libraries(audioviz PUBLIC portaudio)
	endif()

	FetchContent_Declare(portaudio-pp URL https://github.com/trustytrojan/portaudio-pp/archive/main.zip)
	FetchContent_MakeAvailable(portaudio-pp)
	target_compile_definitions(audioviz PUBLIC AUDIOVIZ_PORTAUDIO)
	target_include_directories(audioviz PUBLIC ${portaudio-pp_SOURCE_DIR}/include)
endif()
