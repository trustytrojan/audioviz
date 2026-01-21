file(GLOB_RECURSE LIBAVZ_ANALYSIS_SOURCES src/*.cpp)
add_library(avz-analysis ${LIBAVZ_ANALYSIS_SOURCES})

target_compile_options(avz-analysis PUBLIC -Wno-narrowing -ffast-math)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(avz-analysis PUBLIC
		-Wno-subobject-linkage # caused by tk-spline, safe to ignore
	)
endif()

target_include_directories(avz-analysis PUBLIC
	include
	${CMAKE_CURRENT_BINARY_DIR} # for spline
)

# fftw
if(NOT ANDROID)
	# termux's fftw package is missing FFTW3LibraryDepends.cmake, so don't bother finding
	find_package(FFTW3 COMPONENTS fftw3f QUIET)
endif()
if(WIN32 AND NOT FFTW3_FOUND)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
		message(STATUS "fftw: windows x64 detected: fetching binaries")
		FetchContent_Declare(fftw URL https://fftw.org/pub/fftw/fftw-3.3.5-dll64.zip)
		FetchContent_MakeAvailable(fftw)
		target_link_directories(avz-analysis PUBLIC ${fftw_SOURCE_DIR})
		target_include_directories(avz-analysis PUBLIC ${fftw_SOURCE_DIR})
		target_link_libraries(avz-analysis PUBLIC fftw3f-3)
	else()
		message(STATUS "fftw: windows other architecture (probably ARM64) detected: fetching source")
		set(BUILD_TESTS OFF)
		set(ENABLE_FLOAT ON)
		set(DISABLE_FORTRAN ON)
		FetchContent_Declare(fftw URL https://www.fftw.org/fftw-3.3.10.tar.gz)
		FetchContent_MakeAvailable(fftw)
		target_include_directories(avz-analysis PUBLIC ${fftw_SOURCE_DIR}/api)
		target_link_libraries(avz-analysis PUBLIC fftw3f)
	endif()
else()
	target_link_libraries(avz-analysis PUBLIC fftw3f)
endif()

## spline
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/tk-spline.hpp)
	file(DOWNLOAD https://github.com/ttk592/spline/raw/master/src/spline.h ${CMAKE_CURRENT_BINARY_DIR}/tk-spline.hpp)
endif()
