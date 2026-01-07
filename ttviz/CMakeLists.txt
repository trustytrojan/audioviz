include(deps.cmake)

include_directories(include ${CMAKE_BINARY_DIR})
link_libraries(audioviz)

file(GLOB_RECURSE TTVIZ_SOURCES src/*.cpp)
add_executable(ttviz ${TTVIZ_SOURCES})
