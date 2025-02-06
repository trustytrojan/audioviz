include(deps.cmake)

file(GLOB AUDIOVIZ_SOURCES
	src/fx/*.cpp
	src/media/*.cpp
	src/tt/*.cpp
	src/viz/*.cpp
	src/*.cpp
)

if(AUDIOVIZ_LUA)
	file(GLOB AUDIOVIZ_LUA_SOURCES src/lua/*.cpp)
endif()

add_library(audioviz ${AUDIOVIZ_SOURCES} ${AUDIOVIZ_LUA_SOURCES})
set_target_properties(audioviz PROPERTIES
	PREFIX ""
	OUTPUT_NAME "audioviz"
)

target_include_directories(audioviz
	PUBLIC include
	PUBLIC ${libavpp_SOURCE_DIR}/include
	PUBLIC ${libavpp_SOURCE_DIR}/src
	PUBLIC ${SFML_SOURCE_DIR}/include
	PUBLIC ${CMAKE_BINARY_DIR}
	PUBLIC ${portaudio-pp_SOURCE_DIR}/include
)

target_link_libraries(audioviz
	PUBLIC sfml-graphics
	PUBLIC Boost::process
)
foreach(LIB IN LISTS FFMPEG_LIBS)
	target_link_libraries(audioviz PUBLIC ${LIB})
endforeach()
