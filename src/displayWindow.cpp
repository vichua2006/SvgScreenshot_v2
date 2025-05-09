#define _WIN32_WINNT 0x0603  // Windows 8.1 or later

#include <ShellScalingApi.h>
#include <Windows.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "displayWindow.h"

HBITMAP screenBitmap;  // global bitmap handle; stores screenshot

// taken from: https://stackoverflow.com/questions/54912038/querying-windows-display-scaling
std::pair<double, double> getScalingFactors() {
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
    double horizontalScale = ((double)cxPhysical / (double)cxLogical);
    double verticalScale = ((double)cyPhysical / (double)cyLogical);

    return std::make_pair(horizontalScale, verticalScale);
}

LRESULT CALLBACK ScreenShotWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool isDragging = false;
    static POINT rectStart = {}, rectEnd = {};

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

            // Draw the red rectangle if dragging
            if (isDragging) {
                HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));  // Red pen
                HGDIOBJ hOldPen = SelectObject(hdc, hPen);
                HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));  // No fill

                Rectangle(hdc, rectStart.x, rectStart.y, rectEnd.x, rectEnd.y);

                SelectObject(hdc, hOldPen);
                SelectObject(hdc, hOldBrush);
                DeleteObject(hPen);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            isDragging = true;
            GetCursorPos(&rectStart);
            GetCursorPos(&rectEnd);  // init end point to start point
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (isDragging) {
                GetCursorPos(&rectEnd);
                InvalidateRect(hwnd, NULL, FALSE);  // trigger a repaint of the window
                std::cout << rectEnd.x << ' ' << rectEnd.y << '\n';
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            isDragging = false;
            GetCursorPos(&rectEnd);
            InvalidateRect(hwnd, NULL, false);
        }
        case WM_SETCURSOR: {
            // set cursor to cross
            SetCursor(LoadCursor(NULL, IDC_CROSS));
            return TRUE;
        }
        case WM_KEYDOWN: {
            // Close the window when ESC is pressed
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);  // Posts a WM_QUIT message to exit the message loop
            }
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
    wc.lpfnWndProc = ScreenShotWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc)) {
        std::cerr << "Failed to register window class!" << std::endl;
        return;
    }

    // get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create the window
    HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,  // prevents window appearing in alt+tab list
                               CLASS_NAME,        // Window class
                               "",                // Window title
                               WS_POPUP,          // Window style
                               0, 0, screenWidth, screenHeight,
                               NULL,                   // Parent window
                               NULL,                   // Menu
                               GetModuleHandle(NULL),  // Instance handle
                               NULL                    // Additional application data
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
    int screenWidth = ((double)horizontalScaling * (double)GetSystemMetrics(SM_CXSCREEN));
    int screenHeight = ((double)verticalScaling * (double)GetSystemMetrics(SM_CYSCREEN));

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
