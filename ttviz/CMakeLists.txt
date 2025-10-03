option(TTVIZ_IMGUI_SFML "Use ImGui-SFML to demo a UI for audioviz" OFF)

include(deps.cmake)

include_directories(include ${CMAKE_BINARY_DIR})
link_libraries(audioviz)
if(TTVIZ_IMGUI_SFML)
	link_libraries(ImGui-SFML::ImGui-SFML)
	add_compile_definitions(TTVIZ_IMGUI_SFML)
endif()

file(GLOB_RECURSE TTVIZ_SOURCES src/*.cpp)
add_executable(ttviz ${TTVIZ_SOURCES})
