include(deps.cmake)

file(GLOB LUAVIZ_SOURCES src/*.cpp)

link_libraries(audioviz)
include_directories(include)
add_library(luaviz ${LUAVIZ_SOURCES})

# this will produce luaviz.so instead of libluaviz.so
set_target_properties(luaviz PROPERTIES
    PREFIX ""
)
