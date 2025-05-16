if(WIN32)
    # usual place that lua is installed (via winget or other method)
	set(LUA_PATH "$ENV{LocalAppData}/Programs/Lua")
	if(NOT IS_DIRECTORY "${LUA_PATH}")
		message(FATAL_ERROR "lua not installed!")
		# we can add an option to build lua from source later on
	endif()

	set(LUA_LIB_NAME lua54) # this will need to be updated once lua 5.5 releases
	find_library(LUA_LIB ${LUA_LIB_NAME} PATHS "${LUA_PATH}/lib" REQUIRED)
	find_file(LUA_HEADER "lua.h" PATHS "${LUA_PATH}/include" REQUIRED)

	target_include_directories(luaviz PUBLIC ${LUA_PATH}/include)
	target_link_directories(luaviz PUBLIC ${LUA_PATH}/lib)
	target_link_libraries(luaviz PUBLIC ${LUA_LIB_NAME})
else()
	find_library(LUA_LIB NAMES lua REQUIRED)
	target_link_libraries(luaviz PUBLIC lua)
endif()

FetchContent_Declare(sol2 URL https://github.com/ThePhD/sol2/archive/main.zip)
FetchContent_MakeAvailable(sol2)
target_link_libraries(luaviz PUBLIC sol2)
