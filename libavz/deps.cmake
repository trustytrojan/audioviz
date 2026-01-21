# deps.cmake - fetch, setup, and link to dependencies
include(FetchContent)

## GLEW (caused by pbo optimization changes)
if(WIN32)
	message(STATUS "glew: windows detected: fetching glew binaries")
	FetchContent_Declare(glew URL https://github.com/nigels-com/glew/releases/download/glew-2.3.0/glew-2.3.0-win32.zip)
	FetchContent_MakeAvailable(glew)
	target_include_directories(avz PUBLIC ${glew_SOURCE_DIR}/include)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		target_link_directories(avz PUBLIC ${glew_SOURCE_DIR}/lib/Release/x64)
	else()
		message(FATAL_ERROR "glew: windows other architecture: not implemented, do more work")
	endif()
	target_link_libraries(avz PUBLIC glew32s)
	target_compile_definitions(avz PUBLIC GLEW_STATIC)
else()
	find_package(GLEW REQUIRED)
	target_link_libraries(avz PUBLIC GLEW::GLEW)
endif()

## OpenGL (also caused by optimization changes)
find_package(OpenGL COMPONENTS OpenGL REQUIRED)
target_link_libraries(avz PUBLIC OpenGL::GL)

## sfml
if(WIN32)
	# reduce dynamic linker hassle
	set(SFML_STATIC_LIBRARIES ON CACHE BOOL "")
endif()
find_package(SFML COMPONENTS Graphics Window System QUIET)
if(NOT SFML_FOUND)
	message(STATUS "sfml: FIND_SFML_ERROR: ${FIND_SFML_ERROR}")
	if(WIN32)
		message(STATUS "sfml: windows detected: fetching binaries")
		FetchContent_Declare(sfml URL https://github.com/SFML/SFML/releases/download/3.0.2/SFML-3.0.2-Windows.MinGW.x64.zip)
		FetchContent_MakeAvailable(sfml)
		list(APPEND CMAKE_PREFIX_PATH ${sfml_SOURCE_DIR}/lib/cmake/SFML)
	else()
		message(STATUS "sfml: fetching source")
		set(SFML_BUILD_AUDIO OFF)
		set(SFML_BUILD_NETWORK OFF)
		FetchContent_Declare(sfml URL https://github.com/SFML/SFML/archive/3.0.2.tar.gz)
		FetchContent_MakeAvailable(sfml)
	endif()
endif()
find_package(SFML COMPONENTS Graphics Window System REQUIRED)
target_link_libraries(avz PUBLIC SFML::Graphics)

## spline
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/tk-spline.hpp)
	file(DOWNLOAD https://github.com/ttk592/spline/raw/master/src/spline.h ${CMAKE_CURRENT_BINARY_DIR}/tk-spline.hpp)
endif()

## nlohmann_json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(avz PUBLIC nlohmann_json::nlohmann_json)

### TEMPORARY - libavz should not be responsible for audio playback.
### but to keep things stable i will leave this as is for now.
## portaudio (optional)
if(LIBAVZ_USE_PORTAUDIO)
	FetchContent_Declare(portaudio-pp URL https://github.com/trustytrojan/portaudio-pp/archive/main.tar.gz)
	FetchContent_MakeAvailable(portaudio-pp)
	target_compile_definitions(avz PUBLIC LIBAVZ_PORTAUDIO)
	target_link_libraries(avz PUBLIC portaudio-pp::portaudio-pp)
endif()
