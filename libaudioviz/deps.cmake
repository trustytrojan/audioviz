# deps.cmake - fetch, setup, and link to dependencies
include(FetchContent)

## GLEW (caused by pbo optimization changes)
if(WIN32)
	message("windows detected... fetching glew binaries")
	FetchContent_Declare(glew URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip)
	FetchContent_MakeAvailable(glew)
	target_include_directories(audioviz PUBLIC ${glew_SOURCE_DIR}/include)
	message("CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		target_link_directories(audioviz PUBLIC ${glew_SOURCE_DIR}/lib/Release/x64)
	else()
		message(FATAL_ERROR "other architecture: not implemented, do more work")
	endif()
	target_link_libraries(audioviz PUBLIC glew32s)
	target_compile_definitions(audioviz PUBLIC GLEW_STATIC)
else()
	find_package(GLEW REQUIRED)
	target_link_libraries(audioviz PUBLIC GLEW::GLEW)
endif()

## OpenGL (also caused by optimization changes)
find_package(OpenGL COMPONENTS OpenGL REQUIRED)
target_link_libraries(audioviz PUBLIC OpenGL::GL)

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

		# this is so that we only need to append the build dir to the PATH before running,
		# making that process a bit less error-prone
		file(COPY ${fftw_SOURCE_DIR}/libfftw3f-3.dll DESTINATION ${CMAKE_BINARY_DIR})
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
	FetchContent_Declare(SFML URL https://github.com/SFML/SFML/archive/3.0.2.tar.gz)
	FetchContent_MakeAvailable(SFML)
endif()
target_link_libraries(audioviz PUBLIC SFML::Graphics)

## spline
if(NOT EXISTS ${CMAKE_BINARY_DIR}/tk-spline.hpp)
	file(DOWNLOAD https://github.com/ttk592/spline/raw/master/src/spline.h ${CMAKE_BINARY_DIR}/tk-spline.hpp)
endif()

## nlohmann_json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(audioviz PUBLIC nlohmann_json::nlohmann_json)

### TEMPORARY - libaudioviz should not be responsible for audio playback.
### but to keep things stable i will leave this as is for now.
## portaudio (optional)
if(AUDIOVIZ_USE_PORTAUDIO)
    FetchContent_Declare(portaudio-pp URL https://github.com/trustytrojan/portaudio-pp/archive/main.tar.gz)
    FetchContent_MakeAvailable(portaudio-pp)
    target_compile_definitions(audioviz PUBLIC AUDIOVIZ_PORTAUDIO)
    target_link_libraries(audioviz PUBLIC portaudio-pp::portaudio-pp)
endif()