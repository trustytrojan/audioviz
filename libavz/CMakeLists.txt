file(GLOB_RECURSE LIBAVZ_SOURCES src/avz/*.cpp)

add_library(avz ${LIBAVZ_SOURCES})
include(deps.cmake)

target_compile_options(avz PUBLIC -Wno-narrowing -ffast-math)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(avz PUBLIC
		-Wno-subobject-linkage # caused by tk-spline, safe to ignore
	)

	# for windows mingw gcc 15.2 on the github action runner
	target_link_libraries(avz PUBLIC stdc++exp)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(avz PUBLIC
		-Wno-nan-infinity-disabled # caused by nlohmann/json, safe to ignore
	)
	target_compile_definitions(avz PUBLIC
		_USE_MATH_DEFINES # because spline uses M_PI
	)
endif()

target_include_directories(avz PUBLIC
	include # our public headers
	${CMAKE_CURRENT_BINARY_DIR}
)

# popen() on MinGW expects a 'b' to differentiate text/binary,
# but on Linux it errors if a 'b' is there, so we have to do this...
if(WIN32)
	target_compile_definitions(avz PRIVATE POPEN_R_MODE="rb" POPEN_W_MODE="wb")
else()
	target_compile_definitions(avz PRIVATE POPEN_R_MODE="r" POPEN_W_MODE="w")
endif()

add_subdirectory(analysis)
add_subdirectory(gfx)
target_link_libraries(avz PUBLIC avz-analysis)
target_link_libraries(avz PUBLIC avz-gfx)
