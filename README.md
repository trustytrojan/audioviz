# audioviz
my own audio visualizer, because after effects sucks.
for right now, audioviz only supports linux, but with the libraries i'm using, it shouldn't be hard to port to windows.

here's a demo: https://youtu.be/Avk5lRZb7To

## building
1. install required dependencies
2. run `make`
3. optionally install with `sudo make install`

## dependencies
- [FFmpeg](https://github.com/FFmpeg/FFmpeg)
- [FFTW](https://fftw.org)
- [PortAudio](https://github.com/PortAudio/portaudio)
- [SFML](https://github.com/SFML/SFML)
- [argparse](https://github.com/p-ranav/argparse)
- [glfw](https://github.com/glfw/glfw) (just for getting the display refresh rate; hopefully SFML can do this soon)

## dev note
on namespaces:
- `fx`: post-processing effects
- `tt`: utility & extensions
- `viz`: audio visualizations & components
