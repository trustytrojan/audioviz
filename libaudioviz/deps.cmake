# deps.cmake - fetch, setup, and link to dependencies
include(FetchContent)

## ffmpeg
set(FFMPEG_LIBS avformat avutil avcodec)
find_library(ffmpeg NAMES ${FFMPEG_LIBS})
if(WIN32 AND ffmpeg STREQUAL "ffmpeg-NOTFOUND")
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		find_program(WINGET winget)
		if(WINGET STREQUAL "WINGET-NOTFOUND")
			# we'll download ffmpeg ourselves
			message("winget not found, downloading latest ffmpeg with FetchContent_Declare")
			FetchContent_Declare(ffmpeg URL https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z)
			FetchContent_MakeAvailable(ffmpeg)
			list(APPEND CMAKE_LIBRARY_PATH ${ffmpeg_SOURCE_DIR}/lib)
			target_link_directories(audioviz PUBLIC ${ffmpeg_SOURCE_DIR}/lib)
			target_include_directories(audioviz PUBLIC ${ffmpeg_SOURCE_DIR}/include)
		else()
			# use winget installation of ffmpeg shared libs
			message("winget found!")
			file(GLOB FFMPEG_WINGET_PATH $ENV{LocalAppData}/Microsoft/WinGet/Packages/Gyan.FFmpeg.Shared_Microsoft.Winget.Source_8wekyb3d8bbwe/ffmpeg-*-full_build-shared)
			if(NOT FFMPEG_WINGET_PATH)
				message(FATAL_ERROR "ffmpeg not installed! please install it first.")
			endif()
			message("using ffmpeg path: ${FFMPEG_WINGET_PATH}")
			list(APPEND CMAKE_PREFIX_PATH ${FFMPEG_WINGET_PATH}/lib)
			target_link_directories(audioviz PUBLIC ${FFMPEG_WINGET_PATH}/lib)
			target_include_directories(audioviz PUBLIC ${FFMPEG_WINGET_PATH}/include)
		endif()
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
		message("windows arm64 detected, downloading ffmpeg from https://github.com/tordona/ffmpeg-win-arm64")

		# unfortunately cmake itself can't handle 7zips properly
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

		list(APPEND CMAKE_LIBRARY_PATH ${FFMPEG_PATH}/lib)
		target_link_directories(audioviz PUBLIC ${FFMPEG_PATH}/lib)
		target_include_directories(audioviz PUBLIC ${FFMPEG_PATH}/include)
	endif()
endif()

# find each lib separately as the NAMES parameter only checks for one of the names being present
foreach(LIB IN LISTS FFMPEG_LIBS)
	find_library(${LIB} NAMES ${LIB} REQUIRED)
	target_link_libraries(audioviz PUBLIC ${LIB})
endforeach()

## fftw
find_package(FFTW3 COMPONENTS fftw3f)
if(WIN32 AND NOT fftw3_FOUND)
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

## boost
find_package(Boost COMPONENTS Process Log REQUIRED)
if(NOT Boost_FOUND)
	message("Boost not found, using FetchContent...")
	set(BOOST_INCLUDE_LIBRARIES "process;log")
	FetchContent_Declare(Boost URL https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.7z)
	FetchContent_MakeAvailable(Boost)
	if(WIN32)
		target_link_libraries(audioviz PUBLIC ws2_32) # winsock library required by boost.asio
	endif()
endif()
target_link_libraries(audioviz PUBLIC Boost::process PUBLIC Boost::log)

## header-only libs
FetchContent_Declare(libavpp URL https://github.com/trustytrojan/libavpp/archive/main.zip)
FetchContent_MakeAvailable(libavpp)
target_link_libraries(audioviz PUBLIC libavpp)
file(DOWNLOAD https://github.com/ttk592/spline/raw/master/src/spline.h ${CMAKE_BINARY_DIR}/tk-spline.hpp)

### TEMPORARY - libaudioviz should not be responsible for audio playback.
### but to keep things stable i will leave this as is for now.
## portaudio (optional)
if(AUDIOVIZ_PORTAUDIO)
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
		target_link_libraries(audioviz PUBLIC portaudio)
	endif()

	FetchContent_Declare(portaudio-pp URL https://github.com/trustytrojan/portaudio-pp/archive/main.zip)
	FetchContent_MakeAvailable(portaudio-pp)
	target_compile_definitions(audioviz PUBLIC AUDIOVIZ_PORTAUDIO)
	target_include_directories(audioviz PUBLIC ${portaudio-pp_SOURCE_DIR}/include)
endif()
