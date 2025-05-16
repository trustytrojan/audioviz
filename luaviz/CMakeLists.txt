option(LUAVIZ_EXECUTABLE "Make a luaviz executable that executes a Lua script with the audioviz binding included" OFF)

file(GLOB LUAVIZ_SOURCES src/*.cpp)

if(LUAVIZ_EXECUTABLE)
	list(APPEND LUAVIZ_SOURCES src/exe/main.cpp)
	add_executable(luaviz ${LUAVIZ_SOURCES})
else()
	add_library(luaviz SHARED ${LUAVIZ_SOURCES})

	# this will produce luaviz.so instead of libluaviz.so
	set_target_properties(luaviz PROPERTIES PREFIX "")
endif()

include(deps.cmake)

target_include_directories(luaviz PUBLIC include ${CMAKE_BINARY_DIR})
target_link_libraries(luaviz PUBLIC audioviz)
