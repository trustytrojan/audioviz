# Mesa3D Windows Headless Rendering Setup
# This module automatically downloads and sets up Mesa3D for Windows headless testing
# Reference: https://github.com/pal1000/mesa-dist-win

if(NOT WIN32 OR NOT EXAMPLES_TESTING_USE_MESA3D)
	return()
endif()

# Mesa3D configuration
set(MESA3D_VERSION "25.3.3")
set(MESA3D_URL "https://github.com/pal1000/mesa-dist-win/releases/download/${MESA3D_VERSION}/mesa3d-${MESA3D_VERSION}-release-msvc.7z")
set(MESA3D_SHA256 "66b79057ba273c08daa03551a231995d48b8623bb1efa9f3a7c3e09792d8f1ba")

get_filename_component(MESA3D_ARCHIVE "${MESA3D_URL}" NAME)
get_filename_component(MESA3D_ARCHIVE_DIR "${MESA3D_ARCHIVE}" NAME_WLE)

# Determine architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(MESA3D_ARCH "x64")
else()
	set(MESA3D_ARCH "x86")
endif()

set(MESA3D_ARCHIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${MESA3D_ARCHIVE_DIR}/${MESA3D_ARCHIVE}")
set(MESA3D_ARCH_PATH "${CMAKE_CURRENT_BINARY_DIR}/${MESA3D_ARCHIVE_DIR}/${MESA3D_ARCH}")

# Download and extract Mesa3D if not present
if(NOT EXISTS "${MESA3D_ARCH_PATH}")
	message(STATUS "Downloading Mesa3D ${MESA3D_VERSION} for Windows headless rendering")

	file(DOWNLOAD "${MESA3D_URL}" "${MESA3D_ARCHIVE_PATH}"
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=${MESA3D_SHA256})

	if(NOT EXISTS "${MESA3D_ARCHIVE_PATH}")
		message(FATAL_ERROR "Failed to download Mesa3D from ${MESA3D_URL}")
	endif()

	message(STATUS "Extracting Mesa3D files")
	execute_process(
		COMMAND "${CMAKE_COMMAND}" -E tar x "${MESA3D_ARCHIVE_PATH}" -- ${MESA3D_ARCH}
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${MESA3D_ARCHIVE_DIR}"
		RESULT_VARIABLE EXTRACT_RESULT)

	if(NOT EXTRACT_RESULT EQUAL 0)
		message(FATAL_ERROR "Failed to extract Mesa3D archive")
	endif()

	file(REMOVE "${MESA3D_ARCHIVE_PATH}")
endif()

# Copy Mesa3D DLLs to build output directory
file(GLOB MESA3D_FILES "${MESA3D_ARCH_PATH}/*")

get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

set(MESA3D_INSTALLED_FILES)
foreach(MESA3D_FILE ${MESA3D_FILES})
	get_filename_component(MESA3D_FILE_NAME "${MESA3D_FILE}" NAME)

	if(IS_MULTI_CONFIG)
		list(APPEND MESA3D_INSTALLED_FILES
			"${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${MESA3D_FILE_NAME}")
	else()
		list(APPEND MESA3D_INSTALLED_FILES
			"${CMAKE_CURRENT_BINARY_DIR}/${MESA3D_FILE_NAME}")
	endif()
endforeach()

# Create custom command to copy Mesa3D files
add_custom_command(
	OUTPUT ${MESA3D_INSTALLED_FILES}
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
	${MESA3D_FILES}
	"$<IF:$<BOOL:${IS_MULTI_CONFIG}>,${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>,${CMAKE_CURRENT_BINARY_DIR}>"
	DEPENDS ${MESA3D_FILES})

add_custom_target(setup-mesa3d ALL DEPENDS ${MESA3D_INSTALLED_FILES})