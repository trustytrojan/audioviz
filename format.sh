#!/bin/sh
find examples/ src/ include/ -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i