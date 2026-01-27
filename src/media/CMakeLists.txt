file(GLOB LIBAVZ_MEDIA_SOURCES *.cpp)
add_library(avz-media ${LIBAVZ_MEDIA_SOURCES})
add_library(avz::media ALIAS avz-media)

target_compile_options(avz-media PUBLIC
	-Wno-narrowing
	-ffast-math
	# caused by nlohmann/json, safe to ignore
	$<IF:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>,-Wno-nan-infinity-disabled,>
)

target_include_directories(avz-media PUBLIC
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR} # for json
)

# GLEW (caused by pbo optimization changes)
if(WIN32)
	message(STATUS "[avz-media] glew: windows detected: fetching glew binaries")
	FetchContent_Declare(glew URL https://github.com/nigels-com/glew/releases/download/glew-2.3.0/glew-2.3.0-win32.zip)
	FetchContent_MakeAvailable(glew)
	target_include_directories(avz-media PUBLIC ${glew_SOURCE_DIR}/include)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		target_link_directories(avz-media PUBLIC ${glew_SOURCE_DIR}/lib/Release/x64)
	else()
		message(FATAL_ERROR "[avz-media] glew: windows other architecture: not implemented, do more work")
	endif()
	target_link_libraries(avz-media PUBLIC glew32s)
	target_compile_definitions(avz-media PUBLIC GLEW_STATIC)
else()
	find_package(GLEW REQUIRED)
	target_link_libraries(avz-media PUBLIC GLEW::GLEW)
endif()

# OpenGL (also caused by optimization changes)
find_package(OpenGL COMPONENTS OpenGL REQUIRED)
target_link_libraries(avz-media PUBLIC OpenGL::GL)

# nlohmann_json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)
target_link_libraries(avz-media PUBLIC nlohmann_json::nlohmann_json)

# popen() on MinGW expects a 'b' to differentiate text/binary,
# but on Linux it errors if a 'b' is there, so we have to do this...
target_compile_definitions(avz-media PRIVATE
	POPEN_R_MODE="r$<IF:$<PLATFORM_ID:Windows>,b,>"
	POPEN_W_MODE="w$<IF:$<PLATFORM_ID:Windows>,b,>"
)
