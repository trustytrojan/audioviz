# audioviz
*my own audio visualizer (library), because after effects sucks*

## features
- **low-level** visualizer building
- **layer-and-effect-based** graphics API for **C++ & Lua**
- **super fast** video rendering with ffmpeg
- **faster than adobe after effects!!!!!!!!!**

[here are some songs rendered with audioviz!](https://youtube.com/playlist?list=PLq63g2iq0LVvxNjjoYOL4GMTOdXEdHsBf)

## how things work
below is a great video explaining some of the fundamentals of programming with audio. it will get you up to speed with a decent chunk of the codebase:

[Cinamark - How do computers even render audio...?](https://youtu.be/md79DDofGVo)

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
  - [FFTW3](https://fftw.org)
  - [libavpp](https://github.com/trustytrojan/libavpp) - requires the [FFmpeg](https://ffmpeg.org) libraries
  - `ffmpeg`, the standalone CLI program, part of the [FFmpeg](https://ffmpeg.org) project
  - [portaudio-pp](https://github.com/trustytrojan/portaudio-pp) - requires [PortAudio](https://www.portaudio.com) (the C library, NOT the C++ binding)
  - [SFML 3.0.1](https://github.com/SFML/SFML/tree/3.0.1) - only supports X11 windows
  - [tk-spline](https://github.com/ttk592/spline)
- **ttviz**
  - [argparse](https://github.com/p-ranav/argparse)
  - [ImGui-SFML](https://github.com/SFML/imgui-sfml)
- **luaviz**
  - [sol2](https://github.com/ThePhD/sol2) - requires [Lua](https://lua.org/) 5.x

## project structure
```
audioviz
├── libaudioviz - core library
├── ttviz - first visualizer
├── luaviz - lua binding
├── lua - lua scripts calling the luaviz binding
│ ├── newviz.lua - newer style visualizer using luaviz
│ └── ttviz.lua - recreation of ttviz using luaviz
├── shaders - GLSL shaders used by audioviz::fx classes
└── tests - small test programs for various classes in libaudioviz
```

## developer environment setup
i recommend using vscode as it integrates well with git with no effort. get the [cmake tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) to make development easier.

**on windows:** please use the [mingw toolchain](https://github.com/niXman/mingw-builds-binaries/releases) as it is the only toolchain i have compiled with, and honestly the easiest to setup and use. clangd might freak out about the standard headers being missing: to fix this, open clangd extension settings, and add the following argument:
```
--query-driver=C:\path\to\mingw\bin\g++.exe
```

## todo list / goals
- figure out what to do with the shaders in order to make fully static binaries
- ✅️ `ScopeDrawable` class
  - extra: audio window customization independent of shape size
    - hint: use libswresample from ffmpeg
- 🔄 luaviz: the lua binding
  - 🔄 freshen up api for consumption
  - document the api
- 🔄 interactive gui (long term)
  - ✅️ integrate imgui-sfml into project
  - develop imgui window for each audioviz implementation
- add components/effects based on rhythm
  - involves knowing the tempo of the song
  - want a small translucent flash overlay that flashes to the beat of music
  - copy the osu! stars effect