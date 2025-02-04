include(FetchContent)

option(TTVIZ_LUA "Build ttviz with Lua support via --luafile option" ON)

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

## lua (optional)
if(TTVIZ_LUA)
	if(WIN32)
		# usual place that lua is installed via winget
		set(WINGET_LUA_PATH $ENV{localappdata}/programs/lua)
		list(APPEND CMAKE_PREFIX_PATH ${WINGET_LUA_PATH}/lib)
	endif()
	find_library(lua NAMES lua lua54 REQUIRED)

	file(DOWNLOAD https://github.com/ThePhD/sol2/releases/download/v3.3.0/sol.hpp sol/sol.hpp)
	file(DOWNLOAD https://github.com/ThePhD/sol2/releases/download/v3.3.0/config.hpp sol/config.hpp)
	add_compile_definitions(AUDIOVIZ_LUA)

	if(WIN32)
		include_directories(${WINGET_LUA_PATH}/include)
		link_directories(${WINGET_LUA_PATH}/lib)
		link_libraries(lua54)
	elseif(LINUX)
		link_libraries(lua)
	endif()
endif()
