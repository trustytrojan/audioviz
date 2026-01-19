# Running Audio Visualization Examples on Windows (Headless)

This guide explains how to run the audio visualization examples in a headless Windows environment, such as GitHub Actions CI/CD runners.

## The Challenge

Windows doesn't have a native headless display server like Linux's Xvfb. GUI applications typically require an actual display device. However, using **Mesa3D** software rendering, we can run OpenGL applications without a physical GPU or display.

## Solution: Mesa3D Software Rendering

The testing configuration uses Mesa3D's pre-built MSVC binaries to enable headless rendering. This approach:

- **Requires no C++ code changes** - Mesa3D DLLs are automatically used instead of system OpenGL
- **No GPU needed** - Uses CPU software rendering (slower but sufficient for testing)
- **Automatic setup** - Downloads and caches pre-built Mesa3D binaries
- **Architecture detection** - Automatically uses x64 or x86 binaries

## Usage

### Local Development (Optional)

To test headless rendering locally on Windows:

```bash
cmake -B build -DEXAMPLES_TESTING_USE_MESA3D=ON
cmake --build build
ctest --test-dir build/examples --output-on-failure
```

### GitHub Actions

Add to your workflow file (`.github/workflows/ci.yml`):

```yaml
- name: Run Examples Tests (Windows)
  if: runner.os == 'Windows'
  run: |
    cmake -B build `
      -DEXAMPLES_TESTING_USE_MESA3D=ON `
      -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build/examples --output-on-failure
```

## How It Works

1. **Automatic Download**: On first CMake configuration, Mesa3D binaries are downloaded from the official release
2. **Extraction**: The archive is extracted to the build directory
3. **Dependency Setup**: All test targets automatically depend on the `setup-mesa3d` target
4. **DLL Copying**: Mesa3D DLLs are copied to the executable directory
5. **Environment**: Tests run with `LIBGL_ALWAYS_INDIRECT=1` to ensure software rendering

## Performance Notes

- Mesa3D software rendering is ~10-100x slower than GPU rendering
- Test timeouts are set to 10 seconds per test (appropriate for software rendering)
- Adjust `TIMEOUT` in `testing.cmake` if tests time out

## Troubleshooting

### Tests timeout on CI but not locally
- Increase the `TIMEOUT` property in `testing.cmake`
- CI runners have limited CPU resources

### Mesa3D DLL errors
- Ensure `LIBGL_ALWAYS_INDIRECT=1` is set in test environment
- Check that your OpenGL code doesn't use GPU-specific extensions

### Architecture mismatch
- The CMake script auto-detects 32-bit (x86) vs 64-bit (x64)
- Ensure your build is targeting the correct architecture

## References

- SFML Project's Mesa3D Integration: https://github.com/SFML/SFML/blob/master/cmake/Mesa3D.cmake
- Mesa3D Windows Binaries: https://github.com/pal1000/mesa-dist-win
- Mesa3D Documentation: https://docs.mesa3d.org/
