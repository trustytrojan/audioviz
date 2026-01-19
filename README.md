# audioviz
*my own audio visualizer (library), because after effects sucks*

## features
- **low-level** construction of visuals from audio
- **layer-and-effect** composition
- **super fast** video encoding with ffmpeg
- **faster than adobe after effects!!!!!!!!!**

[here are some songs rendered with audioviz!](https://youtube.com/playlist?list=PLq63g2iq0LVvxNjjoYOL4GMTOdXEdHsBf)

## prerequisite knowledge
below are two great videos by [Cinamark](https://www.youtube.com/@cinamark) explaining some of the fundamentals of programming with audio & visualization:

[How do computers even render audio...?](https://youtu.be/md79DDofGVo)
[How Do Computers Even Visualize Audio...?](https://www.youtube.com/watch?v=cZqle_ukePs)

## building & running

### linux
1. install any required dependencies below
2. run `cmake -S. -Bbuild && cmake --build build -j$(nproc)` or use your IDE of choice with CMake support
3. run build executables from the project root (as explained above)

### windows
1. install any required dependencies below using `winget`:
   - ffmpeg can be installed with `winget install gyan.ffmpeg.shared`
2. run `cmake -S. -Bbuild && cmake --build build` or use your IDE of choice with CMake support
3. run build executables from the project root (as explained above)

#### dll problem
on 64-bit systems, built executables dynamically link to FFTW and PortAudio, which are downloaded by the CMake build system at configure time into the `build` directory (or whatever you passed to `-B` when configuring). so **if nothing happens when you run an audioviz executable from the terminal**, and it returned a non-zero exit code, you need to append `;build` to your `PATH` environment variable so that the libraries can be found. you can do this by running `$env:PATH += ';build'` in PowerShell, or `set PATH=%PATH%;build` in CMD. if you want this to persist for every PowerShell/CMD you open, add the *absolute* path to your `build` directory to your user environment variables.

### macOS
thanks to CI, i discovered that the build process is the same as with linux. just use brew for package management. however i don't own a functional mac at the moment and have **not** tested the CI builds.

## libraries/software used
- **libaudioviz**
  - [FFTW3](https://fftw.org)
  - `ffmpeg`, the standalone CLI program, part of the [FFmpeg](https://ffmpeg.org) project
  - [portaudio-pp](https://github.com/trustytrojan/portaudio-pp)
  - [SFML](https://github.com/SFML/SFML)
  - [tk-spline](https://github.com/ttk592/spline)

## developer environment setup
the project uses C++23 so try to use a compliant toolchain version. for GCC that would be version 14 and above.

i recommend using vscode as it integrates well with git with no effort. get the [cmake tools extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) and [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) to make development easier.

### windows
please use the [mingw toolchain](https://github.com/niXman/mingw-builds-binaries/releases) as it is the only toolchain i have compiled with, and honestly the easiest to setup and use. if you have `winget` you can get the version i usually use with `winget install BrechtSanders.WinLibs.POSIX.UCRT`.

clangd might freak out about the standard headers being missing: to fix this, open clangd extension settings, and add the following argument (**without** quotes around the path):
```
--query-driver=C:\path\to\mingw\bin\g++.exe
```

## todo list / goals
- gonna take imgui out... it belongs in a separate repo as an effort to make some kind of editor based on libaudioviz. ui should not be conflated with the library itself.
- rhythm-based effects
  - use [aubio](https://aubio.org) for this
