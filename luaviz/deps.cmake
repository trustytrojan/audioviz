# make sure we have lua
if(WIN32)
    # usual place that lua is installed via winget
    set(WINGET_LUA_PATH $ENV{localappdata}/programs/lua)
    list(APPEND CMAKE_PREFIX_PATH ${WINGET_LUA_PATH}/lib)
endif()
find_library(lua NAMES lua lua54 REQUIRED)

# get sol2
file(DOWNLOAD https://github.com/ThePhD/sol2/releases/download/v3.3.0/sol.hpp ${CMAKE_BINARY_DIR}/sol/sol.hpp)
file(DOWNLOAD https://github.com/ThePhD/sol2/releases/download/v3.3.0/config.hpp ${CMAKE_BINARY_DIR}/sol/config.hpp)

# put everything together
if(WIN32)
    include_directories(${WINGET_LUA_PATH}/include)
    link_directories(${WINGET_LUA_PATH}/lib)
    link_libraries(lua54)
elseif(LINUX)
    link_libraries(lua)
endif()
