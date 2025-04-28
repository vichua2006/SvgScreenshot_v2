#include <ShellScalingApi.h>
#include <Windows.h>

#include "displayWindow.h"


int main() {
    // make the application DPI (Dot Per Inch) aware
    // basically make windows provide the actual physical dimension of the screen to the program,
    // instead of virtual dimensions caused by scaling
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    captureScreenToBitmap(&screenBitmap);
    displayBitmap();

    DeleteObject(screenBitmap);
    return 0;
}
