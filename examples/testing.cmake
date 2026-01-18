# Enable testing for ctest
enable_testing()

# Set up headless testing with Xvfb
# This script will start Xvfb and run tests with the DISPLAY environment variable set

# Generate test media file if it doesn't exist
add_test(
	NAME generate_test_media
	COMMAND bash -c "
		cd '${CMAKE_CURRENT_BINARY_DIR}'
		if [ ! -f 'panned-sweep-1-20000-5-1.wav' ]; then
			source '${CMAKE_CURRENT_SOURCE_DIR}/generate_freq_sweeps.sh'
			panned_sweep 1 20000 5 1
		fi
	"
)
set_tests_properties(generate_test_media PROPERTIES FIXTURES_SETUP test_media)

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

set(EXAMPLE_MEDIA_FILE "${CMAKE_CURRENT_BINARY_DIR}/panned-sweep-1-20000-5-1.wav")

foreach(example ${EXAMPLES})
	set(EXAMPLE_COMMAND
		gdb --batch --ex r --ex bt --ex q --args
		${CMAKE_CURRENT_BINARY_DIR}/${example}
		--size 100 100
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
