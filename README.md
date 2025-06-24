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

If the build fails with `windows.h: No such file or directory`, run `./setup.sh`
to install the MinGW cross toolchain. The successful build produces
`screenshot.exe` in the repository root. If you prefer to
invoke the compiler manually, use:

```bash
x86_64-w64-mingw32-g++ src/*.cpp -lgdi32 -lole32 -luuid -lcomdlg32 -lshell32 -lmsimg32 -o screenshot.exe
```

Note that compilation on non-Windows hosts still requires a cross compiler such
as `mingw-w64`.

## Screenshots and Electron UI

Captured selections are now written to the `screenshots` directory in the
repository root. Each file is saved as `screenshot_<timestamp>.bmp`.

An example Electron application lives in `electron-app`. It now uses a small
React interface that shows each screenshot in a fixed size card with a button to
copy the image to your clipboard. Run it after installing dependencies with:

```bash
cd electron-app
npm install
npm start
```

When running as the root user (such as in some containers) the Electron runtime
requires the `--no-sandbox` flag. The provided `npm start` script already adds
this flag.

## FastAPI backend

This repository also includes a small FastAPI application for storing screenshot metadata in MongoDB. The server exposes two endpoints to create and list screenshots.

### Running

Install the Python dependencies and start the server with Uvicorn:

```bash
pip install -r requirements.txt
MONGO_URI="<your mongodb atlas uri>" uvicorn backend.main:app --reload
```

The API uses a collection called `screenShots` in the database specified by the `MONGO_DB_NAME` environment variable (defaults to `screenshots_db`).
