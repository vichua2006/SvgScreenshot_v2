#include <windows.h>

#include "displayWindow.h"


int main() {
    // Make the application DPI aware so that Windows provides physical
    // screen dimensions instead of virtual ones caused by scaling.
    // SetProcessDPIAware is widely supported and avoids a dependency on
    // the ShellScalingApi header when cross-compiling.
    SetProcessDPIAware();
    captureScreenToBitmap(&screenBitmap);
    displayBitmap();

    DeleteObject(screenBitmap);
    return 0;
}
