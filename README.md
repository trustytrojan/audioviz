# libavz

_my own audio visualizer library, because after effects sucks_

[here](https://youtube.com/playlist?list=PLq63g2iq0LVvxNjjoYOL4GMTOdXEdHsBf) are
some songs rendered with libavz!

## features

- **low-level** construction of visuals from audio
- **layer-and-effect** composition
- **super fast** video encoding with ffmpeg
- **faster than adobe after effects!!!!!!!!!**

## prerequisite knowledge

below are two great videos by [Cinamark](https://www.youtube.com/@cinamark)
explaining some of the fundamentals of programming with audio & visualization:

[How do computers even render audio...?](https://youtu.be/md79DDofGVo)\
[How Do Computers Even Visualize Audio...?](https://www.youtube.com/watch?v=cZqle_ukePs)

## building

1. install dependencies, per platform:
   ### ubuntu noble (24.04 LTS)
   ```sh
   # ppa required to install SFML 3
   sudo add-apt-repository ppa:bleedingedge/noble-bleed
   sudo apt-get update
   sudo apt-get install -y libfftw3-dev ffmpeg libglew-dev libsfml-dev portaudio19-dev
   ```

   ### arch linux
   ```sh
   sudo pacman -S fftw ffmpeg glew sfml portaudio
   ```

   ### windows
   ```sh
   winget install gyan.ffmpeg
   ```

   ### macOS
   ```sh
   brew install glew sfml fftw ffmpeg
   ```

2. on all platforms: clone repo, configure cmake, and build
   ```sh
   git clone https://github.com/trustytrojan/libavz && cd libavz
   cmake -S. -Bbuild && cmake --build build -j
   ```

3. by default, example programs are built, so you can run them like so:
   ```sh
   build/examples/scope 'my-song.mp3'
   ```

## libraries/software used

- [FFTW3](https://fftw.org): FFT
- [FFmpeg](https://ffmpeg.org): media container transcoding
- [portaudio-pp](https://github.com/trustytrojan/portaudio-pp): C++ header-only
  wrapper for [PortAudio](https://github.com/PortAudio/portaudio), optional
  dependency
- [SFML](https://github.com/SFML/SFML): graphics/windowing
- [tk-spline](https://github.com/ttk592/spline): cubic spline interpolation
- [GLEW](https://github.com/nigels-com/glew): for OpenGL extension loading

## developer environment setup

the project requires C++23 with GNU extensions, so try to use a compliant
toolchain. i primarily use GCC 15.2 on linux and windows. thanks to github
actions, there is success building with AppleClang on macOS (see
[actions workflow runs](https://github.com/trustytrojan/libavz/actions) for
more details). i **do not** guarantee fully working builds with Clang/LLVM or
MSVC. if you want to see them work, contribute!

my choice of editor is [Visual Studio Code](https://code.visualstudio.com/) with
the
[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
and
[clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)
extensions. optionally install
[WebGL GLSL Editor](https://marketplace.visualstudio.com/items?itemName=raczzalan.webgl-glsl-editor)
for shaders.

### windows

please use the [MinGW-w64 toolchain](https://www.mingw-w64.org/) as it is the
only toolchain i have compiled with, and honestly the easiest to setup and use.

with `winget` you can get the version i usually use with the command below. make
sure to restart your terminal if it doesn't pick up on the updated `PATH`
environment variable.

```sh
winget install BrechtSanders.WinLibs.POSIX.UCRT
```

clangd might freak out about the standard headers being missing. to fix this,
open clangd extension settings, and add the following argument **without
quotes**:

```
--query-driver=C:\path\to\mingw\bin\g++.exe
```

## todo list / goals

- gonna take imgui out... it belongs in a separate repo as an effort to make
  some kind of editor using libavz. ui should not be conflated with the library
  itself.
- rhythm-based effects
  - use [aubio](https://aubio.org) for this
