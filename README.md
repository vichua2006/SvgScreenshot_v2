# SvgScreenshot_v2

This is a small experiment for capturing and displaying a screenshot using the Windows API.

## Building

This project targets Windows and requires the Windows SDK. On Debian-based systems you can install a MinGW toolchain and the SDK headers by running the helper script:

```bash
./setup.sh
```

### Using CMake (Recommended)

The project now supports CMake for easier dependency management and cross-platform builds:

```bash
# Create build directory
mkdir build
cd build

# Configure the project
cmake ..

# Build the project
cmake --build .
```

This will produce `screenshot.exe` in the build directory. The CMake configuration automatically:

- Finds and links OpenCV libraries
- Links required Windows GDI libraries
- Sets appropriate compiler flags
- Handles cross-compilation if needed

#### Troubleshooting CMake Issues

If you encounter "nmake not found" or compiler errors:

1. **Specify the generator explicitly:**

   ```bash
   cmake -G "MinGW Makefiles" ..
   ```

2. **Or use Unix Makefiles:**

   ```bash
   cmake -G "Unix Makefiles" ..
   ```

3. **If using Visual Studio:**

   ```bash
   cmake -G "Visual Studio 16 2019" ..
   ```

4. **Check available generators:**
   ```bash
   cmake --help
   ```

### Using Makefile (Legacy)

Alternatively, you can still build using the provided `Makefile`:

```bash
make
```

This will produce `screenshot.exe` in the repository root. If you prefer to
invoke the compiler manually, use:

```bash
x86_64-w64-mingw32-g++ src/*.cpp -lgdi32 -lole32 -luuid -lcomdlg32 -lshell32 -lmsimg32 -o screenshot.exe
```

Note that compilation on non-Windows hosts still requires a cross compiler such
as `mingw-w64`.

## Dependencies

- **OpenCV**: Required for image processing. Install via your system's package manager:
  - Windows: Use vcpkg (`vcpkg install opencv:x64-windows`)
  - Linux: `sudo apt install libopencv-dev`
  - macOS: `brew install opencv`
- **Windows SDK**: Required for Windows API functions
- **MinGW-w64**: For cross-compilation from non-Windows systems
