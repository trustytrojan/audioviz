option(LUAVIZ_EXECUTABLE "Make a luaviz executable that executes a Lua script with the audioviz binding included" OFF)

file(GLOB LUAVIZ_SOURCES src/*.cpp)
add_library(luaviz-intermediate ${LUAVIZ_SOURCES})
target_include_directories(luaviz-intermediate PUBLIC include)
target_link_libraries(luaviz-intermediate PUBLIC audioviz)

include(deps.cmake)

if(LUAVIZ_EXECUTABLE)
	add_executable(luaviz src/exe/main.cpp)
	target_link_libraries(luaviz PUBLIC luaviz-intermediate)
else()
	add_library(luaviz MODULE src/lib/extern.cpp)
	target_link_libraries(luaviz PUBLIC luaviz-intermediate)
	# remove `lib` prefix from filename
	set_target_properties(luaviz PROPERTIES PREFIX "")
endif()
