enable_testing()

option(EXAMPLES_TESTING_USE_GDB "Use GDB when running examples for stacktraces on crashes" OFF)
option(EXAMPLES_TESTING_USE_MESA3D "On Windows, use Mesa3D software rendering for headless testing" OFF)

# Generate test media file if it doesn't exist
set(START_FREQUENCY 1)
set(END_FREQUENCY 20000)
set(PAN_FREQUENCY 1)
set(DURATION_SECONDS 2)
set(SWEEP_EXPRESSION "sin(2*PI*${START_FREQUENCY}*(${DURATION_SECONDS}/log(${END_FREQUENCY}/${START_FREQUENCY}))*(exp(t*log(${END_FREQUENCY}/${START_FREQUENCY})/${DURATION_SECONDS})-1))")
set(FULL_EXPRESSION "aevalsrc='${SWEEP_EXPRESSION}*(0.5+0.5*sin(2*PI*${PAN_FREQUENCY}*t))|${SWEEP_EXPRESSION}*(0.5-0.5*sin(2*PI*${PAN_FREQUENCY}*t))':d=${DURATION_SECONDS}")
set(EXAMPLE_MEDIA_FILE "${CMAKE_CURRENT_BINARY_DIR}/example-audio.wav")
if(NOT EXISTS ${EXAMPLE_MEDIA_FILE})
	add_test(
		NAME generate_test_media
		COMMAND ffmpeg -v warning -f lavfi -i ${FULL_EXPRESSION} -y ${EXAMPLE_MEDIA_FILE}
	)
	set_tests_properties(generate_test_media PROPERTIES FIXTURES_SETUP test_media)
endif()

# Set up Mesa3D for Windows headless rendering
if(WIN32 AND EXAMPLES_TESTING_USE_MESA3D)
	include(${CMAKE_CURRENT_SOURCE_DIR}/mesa3d.cmake)
endif()

if(LINUX)
	# Set up headless testing with Xvfb
	add_test(
		NAME xvfb_start
		COMMAND bash -c "nohup Xvfb :99 -screen 0 100x100x24 -nolisten tcp -noreset -fbdir /dev/shm &>/dev/null & sleep 1"
	)
	set_tests_properties(xvfb_start PROPERTIES FIXTURES_SETUP xvfb_display FIXTURES_REQUIRED test_media)

	add_test(
		NAME xvfb_stop
		COMMAND bash -c "killall Xvfb || true"
	)
	set_tests_properties(xvfb_stop PROPERTIES FIXTURES_CLEANUP xvfb_display)
endif()

foreach(example ${EXAMPLE_PROGRAMS})
	set(EXAMPLE_COMMAND
		${CMAKE_CURRENT_BINARY_DIR}/${example}
		--size 100 100
		--framerate 30
		${EXAMPLE_MEDIA_FILE}
	)

	if(EXAMPLES_TESTING_USE_GDB)
		set(EXAMPLE_COMMAND
			gdb --batch --return-child-result -x ${CMAKE_CURRENT_SOURCE_DIR}/gdb-hook.gdb --args
			${EXAMPLE_COMMAND}
		)
	endif()

	add_test(NAME ${example} COMMAND ${EXAMPLE_COMMAND})

	set(REQUIRED_FIXTURES "test_media")

	if(LINUX)
		list(APPEND REQUIRED_FIXTURES "xvfb_display")
		set(TEST_ENV "DISPLAY=:99")
	endif()

	if(WIN32 AND EXAMPLES_TESTING_USE_MESA3D)
		list(APPEND REQUIRED_FIXTURES "setup-mesa3d")
		set(TEST_ENV "GALLIUM_DRIVER=softpipe")
	endif()

	set_tests_properties(${example} PROPERTIES
		FIXTURES_REQUIRED "${REQUIRED_FIXTURES}"
		ENVIRONMENT "${TEST_ENV}"
		TIMEOUT 10
	)
endforeach()
