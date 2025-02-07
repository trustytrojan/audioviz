include(deps.cmake)

file(GLOB AUDIOVIZ_SOURCES
	src/audioviz/fft/*.cpp
	src/audioviz/fx/*.cpp
	src/audioviz/media/*.cpp
	src/audioviz/*.cpp
)

if(AUDIOVIZ_LUA)
	file(GLOB AUDIOVIZ_LUA_SOURCES src/audioviz/lua/*.cpp)
	message(${AUDIOVIZ_LUA_SOURCES})
endif()

add_library(audioviz ${AUDIOVIZ_SOURCES} ${AUDIOVIZ_LUA_SOURCES})

# this will produce audioviz.so instead of libaudioviz.so
set_target_properties(audioviz PROPERTIES
	PREFIX ""
)

# need to use the target_XXXX commands so that dependents will also include
# these directories and link to the same libraries
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
