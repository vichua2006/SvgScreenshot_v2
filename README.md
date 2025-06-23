# SvgScreenshot_v2

This is a small experiment for capturing and displaying a screenshot using the Windows API.

## Building

This project targets Windows and requires the Windows SDK. On Debian-based systems you can install a MinGW toolchain and the SDK headers by running the helper script:

```bash
./setup.sh
```

After the dependencies are installed, build the project using the provided
`Makefile`:

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
