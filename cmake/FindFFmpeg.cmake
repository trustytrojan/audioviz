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
message("found ffmpeg & ffprobe!")