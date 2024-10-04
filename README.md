# audioviz
my own audio visualizer, because after effects sucks.
for right now, audioviz only supports linux, but with the libraries i'm using, it shouldn't be hard to port to windows.

here are some demos:
- https://youtu.be/Avk5lRZb7To
- https://youtu.be/RaTMz4MPqCM

## building
### linux
1. install any required dependencies below
2. run `cmake -S. -Bbuild && cmake --build build -j$(nproc)` or use your IDE of choice with CMake support

### windows
1. create a directory called `windows_deps`, with three subdirectories `ffmpeg`, `portaudio`, and `fftw`
2. install ffmpeg in one of two ways:
  - run these commands in an **administrator** command prompt (`mklink` doesn't run otherwise), replace `[version]` with the ffmpeg version that was installed by winget:
    ```cmd
    winget install gyan.ffmpeg.shared
    mklink /D windows_deps\ffmpeg %LocalAppData%\Microsoft\WinGet\Packages\Gyan.FFmpeg.Shared_Microsoft.Winget.Source_8wekyb3d8bbwe\ffmpeg-[version]-full_build-shared
    ```
	the `mklink` command simply links the winget installation directory to `windows_deps/ffmpeg`.
  - or, go [here](https://www.gyan.dev/ffmpeg/builds/#release-builds), download the "shared" release and extract the archive to `windows_deps/ffmpeg`
3. download [portaudio](https://files.portaudio.com/download.html) source, extract `portaudio.h` (inside `include` directory of the tarball) to `windows_deps/portaudio`. i recommending using 7zip for viewing archives: you can get it by running `winget install 7zip`.
4. download the [64bit portaudio dll](https://github.com/spatialaudio/portaudio-binaries), rename it to `portaudio_x64.dll`, and move it to `windows_deps/portaudio`
5. download [fftw](https://fftw.org/install/windows.html), open the archive and extract `fftw3.h` and `libfftw3f-3.dll` into `windows_deps/fftw`

## libraries/software used
- [FFTW](https://fftw.org)
- [libavpp](https://github.com/trustytrojan/libavpp)
  - requires the [FFmpeg](https://github.com/FFmpeg/FFmpeg) libraries
- the `ffmpeg` CLI program, also part of the [FFmpeg](https://github.com/FFmpeg/FFmpeg) project
  - required for video encoding
- [portaudio-pp](https://github.com/trustytrojan/portaudio-pp)
  - requires [PortAudio](https://github.com/PortAudio/portaudio) (only the C library)
- [SFML](https://github.com/SFML/SFML)
  - note that SFML only supports X11 windows, so you will need XWayland if you use Wayland
- [argparse](https://github.com/p-ranav/argparse)
- [sol2](https://github.com/ThePhD/sol2)
- [spline](https://github.com/ttk592/spline)

## dev note
on namespaces:
- `fx`: post-processing effects
- `tt`: utility & library extensions
- `viz`: audio visualization components

### environment setup
i recommend using vscode as it integrates well with git with no effort. get the [cmake tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) to make development easier.

on windows please use the [mingw toolchain](https://github.com/niXman/mingw-builds-binaries/releases) as it is the only toolchain i have compiled with, and honestly the easiest to setup and use. clangd might freak out about the standard headers being missing: to fix this, open clangd extension settings, and add the following argument:
```
--query-driver=C:\path\to\mingw\bin\g++.exe
```

## todo list
- `viz::ScopeDrawable`
- lua api (in progress)
  - modular layering/effects system
  - remove hardcoded visualizer components
- make `audioviz` generic, extend it with default functionality in a new class named `ttviz`
  - figure out how to deal with [`fft_size`](src/audioviz.cpp#L284) and [`sa.analyze(...)`](src/audioviz.cpp#L299)
- `clangd` still not configured properly on windows, it doesn't know where to find c++ headers (when using mingw). figure this out
