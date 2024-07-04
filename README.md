# audioviz
my own audio visualizer, because after effects sucks.
for right now, audioviz only supports linux, but with the libraries i'm using, it shouldn't be hard to port to windows.

here's a demo: https://youtu.be/Avk5lRZb7To

## goals/todo
- layering/effects system (in the works)
- configuration files that describe visualizers in either JSON or Lua
- add build flags/`#define`s to allow for video-encode only builds of audioviz

## building
1. install any required dependencies below
2. run `make`

## libraries/software used
- [libavpp](https://github.com/trustytrojan/libavpp)
  - requires the [FFmpeg](https://github.com/FFmpeg/FFmpeg) libraries (`libavutil`, `libavcodec`, `libavformat`, `libswresample`, `libswscale`)
- the `ffmpeg` CLI program, also part of the [FFmpeg](https://github.com/FFmpeg/FFmpeg) project
  - required for video encoding
- [portaudio-pp](https://github.com/trustytrojan/portaudio-pp)
  - requires [PortAudio](https://github.com/PortAudio/portaudio) (only the C library)
- [FFTW](https://fftw.org)
- [SFML](https://github.com/SFML/SFML)
  - specifically [this commit](https://github.com/SFML/SFML/commit/1a40f0195788185d56eca04687a8cd793c90b2fc); this is handled by the [Makefile](/Makefile), so you don't need to install
  - note that SFML only supports X11 windows, so you will need XWayland if you use Wayland
- [argparse](https://github.com/p-ranav/argparse)
  - header-only, included [in this repo](/include/argparse.hpp)
- [glfw](https://github.com/glfw/glfw)
  - just for getting the display refresh rate; hopefully SFML can do this eventually

## dev note
on namespaces:
- `fx`: post-processing effects
- `tt`: utility & library extensions
- `viz`: audio visualization
