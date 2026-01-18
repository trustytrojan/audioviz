# Enable testing for ctest
enable_testing()

# Set up headless testing with Xvfb
# This script will start Xvfb and run tests with the DISPLAY environment variable set

# Generate test media file if it doesn't exist
set(START_FREQUENCY 1)
set(END_FREQUENCY 20000)
set(PAN_FREQUENCY 1)
set(DURATION_SECONDS 2)
set(SWEEP_EXPRESSION "sin(2*PI*${START_FREQUENCY}*(${DURATION_SECONDS}/log(${END_FREQUENCY}/${START_FREQUENCY}))*(exp(t*log(${END_FREQUENCY}/${START_FREQUENCY})/${DURATION_SECONDS})-1))")
set(FULL_EXPRESSION "aevalsrc='${SWEEP_EXPRESSION}*(0.5+0.5*sin(2*PI*${PAN_FREQUENCY}*t))|${SWEEP_EXPRESSION}*(0.5-0.5*sin(2*PI*${PAN_FREQUENCY}*t))':d=${DURATION_SECONDS}")
set(EXAMPLE_MEDIA_FILE "${CMAKE_CURRENT_BINARY_DIR}/example-audio.wav")

add_test(
	NAME generate_test_media
	COMMAND ffmpeg -f lavfi -i ${FULL_EXPRESSION} ${EXAMPLE_MEDIA_FILE}
)
set_tests_properties(generate_test_media PROPERTIES FIXTURES_SETUP test_media)

if(LINUX)
	add_test(
		NAME xvfb_start
		COMMAND bash -c "nohup Xvfb :99 -screen 0 100x100x24 -nolisten tcp -noreset -fbdir /dev/shm > /dev/null 2>&1 & sleep 1"
	)
	set_tests_properties(xvfb_start PROPERTIES FIXTURES_SETUP xvfb_display FIXTURES_REQUIRED test_media)

	add_test(
		NAME xvfb_stop
		COMMAND bash -c "killall Xvfb || true"
	)
	set_tests_properties(xvfb_stop PROPERTIES FIXTURES_CLEANUP xvfb_display)
endif()

# Define example test cases
set(EXAMPLES
	scope
	polar-spectrum
	shake-bass
	ranged-spectrum
	stereo-scope
	stereo-polar-spectrum
	old-bass-nation
	mirrored-bass-nation
	# spectrum-new-api
)

foreach(example ${EXAMPLES})
	set(EXAMPLE_COMMAND
		gdb --batch --return-child-result \
			--ex "set pagination off" \
			--ex "set backtrace limit 0" \
			--ex "run" \
			--ex "thread apply all bt full" \
			--ex "quit" --args \
		${CMAKE_CURRENT_BINARY_DIR}/${example} \
		--size 100 100 \
		${EXAMPLE_MEDIA_FILE}
	)
	add_test(
		NAME ${example}
		COMMAND ${EXAMPLE_COMMAND}
	)
	set_tests_properties(${example} PROPERTIES
		FIXTURES_REQUIRED "xvfb_display;test_media"
		ENVIRONMENT "DISPLAY=:99"
	)
endforeach()
