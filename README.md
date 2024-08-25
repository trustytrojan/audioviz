# audioviz
my own audio visualizer, because after effects sucks.
for right now, audioviz only supports linux, but with the libraries i'm using, it shouldn't be hard to port to windows.

here's a demo: https://youtu.be/Avk5lRZb7To

## building
1. install any required dependencies below
2. run `cmake -S. -Bbuild && cmake --build build -j$(nproc)` or use your IDE of choice with CMake support

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

## todo list
- `viz::ScopeDrawable`
- lua api (in progress)
  - modular layering/effects system
  - remove hardcoded visualizer components
- make `viz::ParticleSystem` more customizable
  - add `set_displacement_direction(sf::Vector2f)`
  - replace `update(sf::Vector2f)` with `displace_particles(float)`
