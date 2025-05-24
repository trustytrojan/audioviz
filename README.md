# audioviz
my own audio visualizer library, because after effects sucks.

## features
- **low-level** visualizer building
- **layer-based** graphics API for **C++ & Lua**
- **super fast** video rendering with ffmpeg
- **faster than adobe after effects!!!!!!!!!**

here are some songs **rendered** with audioviz!
- [hyperforms - LOSE CONTROL](https://youtu.be/Avk5lRZb7To)
- [Chase Atlantic - DIE FOR ME](https://youtu.be/RaTMz4MPqCM)

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
  - [SFML 3.0.1](https://github.com/SFML/SFML/tree/3.0.1) - only supports X11 windows
  - [tk-spline](https://github.com/ttk592/spline)
- **ttviz**
  - [argparse](https://github.com/p-ranav/argparse)
  - [ImGui-SFML](https://github.com/SFML/imgui-sfml)
- **luaviz**
  - [sol2](https://github.com/ThePhD/sol2) - requires [Lua](https://lua.org/) 5.4

## developer environment setup
i recommend using vscode as it integrates well with git with no effort. get the [cmake tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) to make development easier.

**on windows:** please use the [mingw toolchain](https://github.com/niXman/mingw-builds-binaries/releases) as it is the only toolchain i have compiled with, and honestly the easiest to setup and use. clangd might freak out about the standard headers being missing: to fix this, open clangd extension settings, and add the following argument:
```
--query-driver=C:\path\to\mingw\bin\g++.exe
```

## dev note
on namespaces:
- `audioviz`: namespace for all classes in **libaudioviz**
- `audioviz::fft`: classes that wrap FFTW
- `audioviz::fx`: post-processing effects for `audioviz::Layer`
- `audioviz::media`: media provider implementations

## todo list / goals
- ✅️ `ScopeDrawable` class
  - extra: audio window customization independent of shape size
- 🔄 lua api: **luaviz**
  - ✅️ modular layering/effects system
  - ✅️ remove hardcoded visualizer components
  - 🔄 freshen up api for consumption
  - ✅️ decide whether to split up audioviz into library & programs
  - document the api
- add components/effects based on rhythm
  - involves knowing the tempo of the song
  - want a small translucent flash overlay that flashes to the beat of music
  - copy the osu! stars effect
- 🔄 interactive gui (long term)
  - ✅️ integrate imgui-sfml into project
  - develop imgui window for each audioviz implementation