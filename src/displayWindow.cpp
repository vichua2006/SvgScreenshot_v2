#define _WIN32_WINNT 0x0603  // Windows 8.1 or later

#include "displayWindow.h"

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <utility>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

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

        // Create a memory DC for the screenshot
        HDC hdcMemory = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemory, screenBitmap);

        // Get the bitmap dimensions
        BITMAP bitmap;
        GetObject(screenBitmap, sizeof(BITMAP), &bitmap);

        // Create an off-screen buffer to avoid flicker when redrawing
        HDC hdcBuffer = CreateCompatibleDC(hdc);
        HBITMAP hBufferBitmap = CreateCompatibleBitmap(hdc, bitmap.bmWidth, bitmap.bmHeight);
        HBITMAP hOldBufferBitmap = (HBITMAP)SelectObject(hdcBuffer, hBufferBitmap);

        // Draw the screenshot into the buffer first
        BitBlt(hdcBuffer, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMemory, 0, 0, SRCCOPY);

        // Create a semi-transparent overlay to darken the entire screen
        HDC hdcOverlay = CreateCompatibleDC(hdc);
        HBITMAP hOverlayBitmap =
            CreateCompatibleBitmap(hdc, bitmap.bmWidth, bitmap.bmHeight);
        HBITMAP hOldOverlayBitmap = (HBITMAP)SelectObject(hdcOverlay, hOverlayBitmap);
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
        RECT fullRect = { 0, 0, bitmap.bmWidth, bitmap.bmHeight };
        FillRect(hdcOverlay, &fullRect, hBrush);

        BLENDFUNCTION blend = { AC_SRC_OVER, 0, 128, 0 };  // 50% opacity
        AlphaBlend(hdcBuffer, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcOverlay, 0, 0,
            bitmap.bmWidth, bitmap.bmHeight, blend);

        // Draw the red rectangle and darken the area outside of it if dragging
        if (isDragging) {
            // Normalize the rectangle coordinates in case the user drags in any direction
            int left = std::min(rectStart.x, rectEnd.x);
            int top = std::min(rectStart.y, rectEnd.y);
            int right = std::max(rectStart.x, rectEnd.x);
            int bottom = std::max(rectStart.y, rectEnd.y);

            // Restore the screenshot brightness in the selected area
            BitBlt(hdcBuffer, left, top, right - left, bottom - top, hdcMemory, left, top, SRCCOPY);

            // Draw the red rectangle outline
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
            HGDIOBJ hOldPen = SelectObject(hdcBuffer, hPen);
            HGDIOBJ hOldBrush = SelectObject(hdcBuffer, GetStockObject(NULL_BRUSH));
            Rectangle(hdcBuffer, left, top, right, bottom);
            SelectObject(hdcBuffer, hOldPen);
            SelectObject(hdcBuffer, hOldBrush);
            DeleteObject(hPen);
            DeleteDC(hdcOverlay);
        }

        // Copy the composed buffer to the window in one operation
        BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcBuffer, 0, 0, SRCCOPY);

        // Cleanup
        SelectObject(hdcOverlay, hOldOverlayBitmap);
        DeleteObject(hOverlayBitmap);
        DeleteObject(hBrush);
        DeleteDC(hdcOverlay);

        SelectObject(hdcBuffer, hOldBufferBitmap);
        DeleteObject(hBufferBitmap);
        DeleteDC(hdcBuffer);

        SelectObject(hdcMemory, hOldBitmap);
        DeleteDC(hdcMemory);

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
        InvalidateRect(hwnd, NULL, FALSE);
        copySelectionToClipboard(rectStart, rectEnd);
        PostQuitMessage(0);
        return 0;
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
void captureScreenToBitmap(HBITMAP* hBitmap) {
    // Get the device context of the entire screen
    HDC hdcScreen = GetDC(NULL);

    // Get the screen dimensions and scale to correct size
    auto [horizontalScaling, verticalScaling] = getScalingFactors();
    int screenWidth = ((double)horizontalScaling * (double)GetSystemMetrics(SM_CXSCREEN));
    int screenHeight = ((double)verticalScaling * (double)GetSystemMetrics(SM_CYSCREEN));

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

static void saveBitmapToFile(HBITMAP hBitmap, const std::wstring& path) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi{};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
    std::vector<BYTE> pixels(dwBmpSize);

    HDC hdc = GetDC(NULL);
    GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pixels.data(), reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    BITMAPFILEHEADER bmfHeader{};
    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwBmpSize + bmfHeader.bfOffBits;

    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD dwWritten = 0;
    WriteFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwWritten, NULL);
    WriteFile(hFile, &bi, sizeof(bi), &dwWritten, NULL);
    WriteFile(hFile, pixels.data(), dwBmpSize, &dwWritten, NULL);
    CloseHandle(hFile);
}

// Copies the selected area from the global screenBitmap to the clipboard and saves it to disk.
void copySelectionToClipboard(POINT start, POINT end) {
    int left = std::min(start.x, end.x);
    int top = std::min(start.y, end.y);
    int width = std::abs(end.x - start.x);
    int height = std::abs(end.y - start.y);
    if (width == 0 || height == 0) {
        return;
    }

    HDC hdcScreen = GetDC(NULL);
    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
    HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, screenBitmap);

    HDC hdcDst = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmCrop = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOldDst = (HBITMAP)SelectObject(hdcDst, hbmCrop);

    BitBlt(hdcDst, 0, 0, width, height, hdcSrc, left, top, SRCCOPY);

    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDst, hOldDst);
    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    ReleaseDC(NULL, hdcScreen);

    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hbmCrop);
        CloseClipboard();
    }
    else {
        DeleteObject(hbmCrop);
    }

    CreateDirectoryA("screenshots", NULL);
    auto now = std::chrono::system_clock::now();
    auto ts = std::chrono::system_clock::to_time_t(now);
    wchar_t path[MAX_PATH];
    swprintf(path, L"./screenshots/screenshot_%lld.bmp", (long long)ts);
    saveBitmapToFile(hbmCrop, path);
}
