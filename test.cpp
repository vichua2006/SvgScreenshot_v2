#define _WIN32_WINNT 0x0603 // Windows 8.1 or later

#include <Windows.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <ShellScalingApi.h>

HBITMAP screenBitmap; // global bitmap handle; stores screenshot

void printCursorPos() {
    POINT p;
    GetCursorPos(&p); // function takes a pointer/address to a POINT struct
    std::cout << p.x << " " << p.y << std::endl;
}

// taken from: https://stackoverflow.com/questions/54912038/querying-windows-display-scaling
// TODO: add denpendency management to this probject
std::pair<double, double> getScalingFactors(){
    auto activeWindow = GetActiveWindow();
    HMONITOR monitor = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);

    // Get the logical width and height of the monitor
    MONITORINFOEX monitorInfoEx;
    monitorInfoEx.cbSize = sizeof(monitorInfoEx);
    GetMonitorInfo(monitor, &monitorInfoEx);
    auto cxLogical = monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left;
    auto cyLogical = monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top;

    // Get the physical width and height of the monitor
    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);
    devMode.dmDriverExtra = 0;
    EnumDisplaySettings(monitorInfoEx.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
    auto cxPhysical = devMode.dmPelsWidth;
    auto cyPhysical = devMode.dmPelsHeight;

    // Calculate the scaling factor
    double horizontalScale = ((double) cxPhysical / (double) cxLogical);
    double verticalScale = ((double) cyPhysical / (double) cyLogical);

    return std::make_pair(horizontalScale, verticalScale);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Create a memory DC and select the bitmap into it
        HDC hdcMemory = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, screenBitmap);

        // Get the bitmap dimensions
        BITMAP bitmap;
        GetObject(screenBitmap, sizeof(BITMAP), &bitmap);

        // Draw the bitmap onto the window
        BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMemory, 0, 0, SRCCOPY);

        // Cleanup
        SelectObject(hdcMemory, hOldBitmap);
        DeleteDC(hdcMemory);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Function to display a window with the contents of a bitmap
void displayBitmap() {
    // Define the Window Procedure

    // Register the window class
    const char CLASS_NAME[] = "BitmapWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class!" << std::endl;
        return;
    }

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        "",                             // Window title
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,   // Position
        CW_USEDEFAULT, CW_USEDEFAULT,   // Size
        NULL,                           // Parent window
        NULL,                           // Menu
        GetModuleHandle(NULL),          // Instance handle
        NULL                            // Additional application data
    );

    if (!hwnd) {
        std::cerr << "Failed to create window!" << std::endl;
        return;
    }

    // Show the window
    ShowWindow(hwnd, SW_SHOW);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// mutates the given bitmap
void captureScreenToBitmap(HBITMAP *hBitmap) {
    // Get the device context of the entire screen
    HDC hdcScreen = GetDC(NULL);

    // Get the screen dimensions and scale to correct size
    auto [horizontalScaling, verticalScaling] = getScalingFactors();
    int screenWidth = ((double) horizontalScaling * (double) GetSystemMetrics(SM_CXSCREEN));
    int screenHeight = ((double) verticalScaling * (double) GetSystemMetrics(SM_CYSCREEN));

    std::cout << screenWidth << '\n' << screenHeight << std::endl;

    // Create a memory DC compatible with the screen
    HDC hdcMemory = CreateCompatibleDC(hdcScreen);

    // Create a compatible bitmap for the screen
    *hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    if (!*hBitmap) {
        std::cerr << "Failed to create compatible bitmap!" << std::endl;
        DeleteDC(hdcMemory);
        ReleaseDC(NULL, hdcScreen);
    }

    // Select the bitmap into the memory DC
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, *hBitmap);

    // Copy the screen content into the memory DC
    if (!BitBlt(hdcMemory, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY)) {
        std::cerr << "BitBlt failed!" << std::endl;
        SelectObject(hdcMemory, hOldBitmap);
        DeleteObject(*hBitmap);
        DeleteDC(hdcMemory);
        ReleaseDC(NULL, hdcScreen);
    }

    // Restore the original bitmap in the memory DC
    SelectObject(hdcMemory, hOldBitmap);

    // Cleanup
    DeleteDC(hdcMemory);
    ReleaseDC(NULL, hdcScreen);
}

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