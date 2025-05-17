option(LUAVIZ_EXECUTABLE "Make luaviz an executable that executes a Lua script with the audioviz binding included" OFF)

file(GLOB LUAVIZ_SOURCES src/*.cpp)

if(LUAVIZ_EXECUTABLE)
	add_executable(luaviz ${LUAVIZ_SOURCES} src/exe/main.cpp)
else()
	add_library(luaviz MODULE ${LUAVIZ_SOURCES} src/lib/extern.cpp)
	set_target_properties(luaviz PROPERTIES PREFIX "") # remove `lib` prefix from filename
endif()

include(deps.cmake)

target_include_directories(luaviz PUBLIC include)
target_link_libraries(luaviz PUBLIC audioviz)

audioviz_compile_definitions(luaviz)
