include(FetchContent)

## dear imgui & imgui-sfml
FetchContent_Declare(
	imgui
	URL https://github.com/ocornut/imgui/archive/v1.91.8.zip
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML FALSE CACHE BOOL "" FORCE)
set(IMGUI_SFML_IMGUI_DEMO FALSE CACHE BOOL "" FORCE)
FetchContent_Declare(
	imgui-sfml
	URL https://github.com/SFML/imgui-sfml/archive/v3.0.zip
)

FetchContent_MakeAvailable(imgui imgui-sfml)
