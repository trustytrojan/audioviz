# audioviz
my own audio visualizer, because after effects sucks.
builds on windows and linux!!!!

here are some demos:
- https://youtu.be/Avk5lRZb7To
- https://youtu.be/RaTMz4MPqCM

## building
### linux
1. install any required dependencies below
2. run `cmake -S. -Bbuild && cmake --build build -j$(nproc)` or use your IDE of choice with CMake support

### windows
1. install any required dependencies below using `winget`:
   - lua can be installed with `winget install devcom.lua`
   - ffmpeg and its libraries can be installed with `winget install gyan.ffmpeg.shared`
2. run `cmake -S. -Bbuild && cmake --build build` or use your IDE of choice with CMake support

## libraries/software used
- [FFTW](https://fftw.org)
- [libavpp](https://github.com/trustytrojan/libavpp)
  - requires the [FFmpeg](https://ffmpeg.org) libraries
- the `ffmpeg` CLI program, also part of the [FFmpeg](https://ffmpeg.org) project
  - required for video encoding
- [portaudio-pp](https://github.com/trustytrojan/portaudio-pp)
  - requires [PortAudio](https://www.portaudio.com) (only the C library)
- [SFML 3.0.0-rc.1](https://github.com/SFML/SFML/tree/3.0.0-rc.1)
  - note that SFML only supports X11 windows, so you will need XWayland if you use Wayland
- [Boost.Process](https://github.com/boostorg/process)
  - included in [Boost 1.86.0](https://github.com/boostorg/boost/releases/tag/boost-1.86.0) (which is what we use now) or higher
- [argparse](https://github.com/p-ranav/argparse)
- [sol2](https://github.com/ThePhD/sol2)
- [spline](https://github.com/ttk592/spline)

## dev note
on namespaces:
- `fx`: post-processing effects
- `tt`: utility & library extensions
- `viz`: audio visualization components
- `media`: media provider implementations

### environment setup
i recommend using vscode as it integrates well with git with no effort. get the [cmake tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) to make development easier.

on windows please use the [mingw toolchain](https://github.com/niXman/mingw-builds-binaries/releases) as it is the only toolchain i have compiled with, and honestly the easiest to setup and use. clangd might freak out about the standard headers being missing: to fix this, open clangd extension settings, and add the following argument:
```
--query-driver=C:\path\to\mingw\bin\g++.exe
```

## todo list / goals
- ✅️ `viz::ScopeDrawable`
  - extra: audio window customization independent of shape size
- 🔄 lua api
  - ✅️ modular layering/effects system
  - ✅️ remove hardcoded visualizer components
  - freshen up c++ api for consumption
  - decide whether to split up audioviz into library & programs
- ✅️ make `audioviz` generic, extend it with default functionality in a new class named `ttviz`
  - ✅️ figure out how to deal with `fft_size` and `sa.analyze(...)`
  - extra: rename `base_audioviz` to `audioviz` and call the default implementation something else
- ✅️ need to experiment with only using the ffmpeg CLI instead of the libraries
  - ✅️ figure out metadata parsing, then subprocessing for the audio/video streams
  - ✅️ this *might* fix all the "ending early" problems (not a guarantee)
  - extra: try using `basic_ipstream` over `basic_pipe`
  - extra: try to have one `ffmpeg` process output both streams
- add components/effects based on rhythm
  - involves knowing the tempo of the song
  - want a small translucent flash overlay that flashes to the beat of music
  - copy the osu! stars effect
- interactive gui (long term)
- libaudioviz - turn this project into a set of tools rather than one binary
  - separate core code into a library "libaudioviz"
  - separate the main features (realtime visualizer, video renderer, lua binding) into their own programs/libraries
