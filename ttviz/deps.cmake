include(FetchContent)

# argparse
if(NOT EXISTS ${CMAKE_BINARY_DIR}/argparse.hpp)
	file(DOWNLOAD https://github.com/p-ranav/argparse/raw/master/include/argparse/argparse.hpp ${CMAKE_BINARY_DIR}/argparse.hpp)
endif()
