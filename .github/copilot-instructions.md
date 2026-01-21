# libavz Copilot Instructions

## Project Overview
libavz is a C++23 audio visualization library that focuses on layer-and-effect composition, built on top of SFML, FFTW3, and FFmpeg.

## Core Architecture
- **`avz::Base`**: The main entry point for a visualizer. Subclass this to define your visuals. It manages a stack of `avz::Layer` objects.
- **`avz::Layer`**: Represents a visual layer. Layers contain `avz::DrawCall`s.
- **`avz::DrawCall`**: Pairs an `sf::Drawable` with an optional `avz::fx::TransformEffect`.
- **`avz::Player`**: Handles the render loop, either in an interactive window or by encoding to a video file using FFmpeg.
- **`avz::media::Media`**: Abstraction for reading audio data (e.g., `FfmpegPopenMedia`).

## Essential Workflows
- **Build**: `cmake -S. -Bbuild && cmake --build build -j`
- **Run Examples**: `build/examples/basic-spectrum 'path/to/music.mp3'`
- **Adding a Visual**: 
  1. Subclass `avz::Base` or `avz::examples::ExampleBase`.
  2. Implement `update(std::span<const float> audio_buffer)`.
  3. Use `emplace_layer<avz::Layer>("name")` to add layers.
  4. Use `avz::util::extract_channel` if processing stereo audio.

## Patterns and Conventions
- **C++23 & GNU Extensions**: Uses modern C++ features. Prefer `std::span` for audio buffers.
- **Shader Integration**: Shaders in `libavz/shaders/` are automatically converted to C++ headers by CMake.
  - Access them via `libavz_shader_<filename_with_underscores>`.
  - See `avz::fx::Polar` for a pattern of how to load and use these shaders.
- **Performance Profiling**: Use the `capture_time(label, code)` macro to measure execution time of blocks within `update()`.
- **Memory Management**: many components use `avz::aligned_allocator` for SIMD-friendly FFT operations.
- **Layer Management**: `Base::emplace_layer` returns a raw pointer; `Base` owns the layer via `std::unique_ptr`.

## Integration Points
- **SFML**: core graphics and windowing.
- **FFTW3**: handled via `avz::AudioAnalyzer` and `avz::FrequencyAnalyzer`.
- **FFmpeg**: handled via `FfmpegPopenMedia` and `FfmpegPopenEncoder`.
- **portaudio-pp**: (Optional) Used by `Player` for real-time audio playback during windowed preview.

## Key Files to Reference
- `libavz/include/avz/Base.hpp`: Core visualizer class.
- `libavz/include/avz/Layer.hpp`: Layer and DrawCall definitions.
- `examples/include/ExampleFramework.hpp`: Helper macros and base classes for example visualizers.
- `libavz/include/avz/fx/TransformEffect.hpp`: Interface for shader-based effects.
- `libavz/include/avz/fft/AudioAnalyzer.hpp`: High-level FFT wrapper.
