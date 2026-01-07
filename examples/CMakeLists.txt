link_libraries(audioviz)
add_executable(stereo-scope stereo-scope.cpp)
add_executable(stereo-spectrum stereo-spectrum.cpp)
# add_executable(glow-test glow-test.cpp)

if(AUDIOVIZ_USE_IMGUI)
	add_executable(spectrum-imgui spectrum-imgui.cpp)
	add_executable(scope-imgui scope-imgui.cpp)
endif()