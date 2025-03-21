include(deps.cmake)

link_libraries(audioviz)
include_directories(include ${CMAKE_BINARY_DIR})

file(GLOB LUAVIZ_SOURCES src/*.cpp)
add_library(luaviz ${LUAVIZ_SOURCES})

# this will produce luaviz.so instead of libluaviz.so
set_target_properties(luaviz PROPERTIES
    PREFIX ""
)
