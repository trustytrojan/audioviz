include(deps.cmake)

include_directories(include ${CMAKE_BINARY_DIR})
link_libraries(audioviz ImGui-SFML::ImGui-SFML)

if(AUDIOVIZ_PORTAUDIO)
	add_compile_definitions(AUDIOVIZ_PORTAUDIO)
endif()

file(GLOB_RECURSE TTVIZ_SOURCES src/*.cpp)
add_executable(ttviz ${TTVIZ_SOURCES})
