#!/bin/sh
find examples/ libavz/ -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i