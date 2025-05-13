include(FetchContent)

# argparse
file(
	DOWNLOAD
	https://github.com/p-ranav/argparse/raw/master/include/argparse/argparse.hpp
	${CMAKE_BINARY_DIR}/argparse.hpp
)

# imgui
FetchContent_Declare(imgui URL https://github.com/ocornut/imgui/archive/v1.91.8.zip)
FetchContent_MakeAvailable(imgui)

# imgui-sfml (depends on imgui being downloaded manually)
find_package(SFML COMPONENTS Graphics)
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_IMGUI_DEMO OFF)
FetchContent_Declare(imgui-sfml URL https://github.com/SFML/imgui-sfml/archive/v3.0.zip)
FetchContent_MakeAvailable(imgui-sfml)
