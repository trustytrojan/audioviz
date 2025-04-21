# audioviz
my own audio visualizer library, because after effects sucks.

**main features:**
- **low-level** visualizer building
- **layer-based** graphics API for **C++ & Lua**
- **super fast** video rendering with ffmpeg (with hardware encoding)

here are some songs **rendered** with audioviz!
- https://youtu.be/Avk5lRZb7To
- https://youtu.be/RaTMz4MPqCM

## building
### linux
1. install any required dependencies below
2. run `cmake -S. -Bbuild && cmake --build build -j$(nproc)` or use your IDE of choice with CMake support

### windows
1. install any required dependencies below using `winget`:
   - lua can be installed with `winget install devcom.lua`
   - ffmpeg can be installed with `winget install gyan.ffmpeg.shared`
2. run `cmake -S. -Bbuild && cmake --build build` or use your IDE of choice with CMake support

### macOS
havent tried building just yet because of lack of native opengl support (SFML requirement). but opengl -> metal compatibility layers exist; building/testing contributions are welcome!

## libraries/software used
- **libaudioviz**
  - [FFTW](https://fftw.org)
  - [libavpp](https://github.com/trustytrojan/libavpp) - requires the [FFmpeg](https://ffmpeg.org) libraries
  - the `ffmpeg` CLI program, also part of the [FFmpeg](https://ffmpeg.org) project
  - [portaudio-pp](https://github.com/trustytrojan/portaudio-pp) - requires [PortAudio](https://www.portaudio.com) (only the C library)
  - [SFML 3.0.0](https://github.com/SFML/SFML/tree/3.0.0-rc.1) - only supports X11 windows
  - [Boost.Process](https://github.com/boostorg/process) - included in [Boost 1.86.0](https://github.com/boostorg/boost/releases/tag/boost-1.86.0) (which is what we use now) or higher
  - [tk-spline](https://github.com/ttk592/spline)
- **ttviz**
  - [argparse](https://github.com/p-ranav/argparse)
  - [ImGui-SFML](https://github.com/SFML/imgui-sfml)
  - Boost.Process
- **luaviz**
  - [sol2](https://github.com/ThePhD/sol2) - requires Lua (preferably 5.4)

## developer environment setup
i recommend using vscode as it integrates well with git with no effort. get the [cmake tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) to make development easier.

**on windows:** please use the [mingw toolchain](https://github.com/niXman/mingw-builds-binaries/releases) as it is the only toolchain i have compiled with, and honestly the easiest to setup and use. clangd might freak out about the standard headers being missing: to fix this, open clangd extension settings, and add the following argument:
```
--query-driver=C:\path\to\mingw\bin\g++.exe
```

## dev note
on namespaces:
- `audioviz`: namespace for all classes in **libaudioviz**
- `audioviz::fx`: post-processing effects for `audioviz::Layer`s
- `audioviz::media`: media provider implementations

## todo list / goals
- ✅️ `ScopeDrawable` class
  - extra: audio window customization independent of shape size
- 🔄 lua api: **luaviz**
  - ✅️ modular layering/effects system
  - ✅️ remove hardcoded visualizer components
  - 🔄 freshen up api for consumption
  - ✅️ decide whether to split up audioviz into library & programs
- ✅️ make `audioviz` generic, extend it with default functionality in a new class named `ttviz`
  - ✅️ figure out how to deal with `fft_size` and `sa.analyze(...)`
  - ✅️ extra: rename `base_audioviz` to `audioviz` and call the default implementation something else
- ✅️ need to experiment with only using the ffmpeg CLI instead of the libraries
  - ✅️ figure out metadata parsing, then subprocessing for the audio/video streams
  - ✅️ this *might* fix all the "ending early" problems (not a guarantee)
  - extra: try using `basic_ipstream` over `basic_pipe`
  - extra: try to have one `ffmpeg` process output both streams (seems impossible due to boost::process limitations)
- add components/effects based on rhythm
  - involves knowing the tempo of the song
  - want a small translucent flash overlay that flashes to the beat of music
  - copy the osu! stars effect
- 🔄 interactive gui (long term)
  - ✅️ integrate imgui-sfml into project
  - develop imgui window for each audioviz implementation
- 🔄 libaudioviz - turn this project into a set of tools rather than one binary
  - ✅️ separate core code into a library "libaudioviz"
  - 🔄 separate the main features (realtime visualizer, video renderer, lua binding) into their own programs/libraries
  - ✅️ put everything in one namespace `audioviz` (except for very interrelated classes like `fx` and `media`)
