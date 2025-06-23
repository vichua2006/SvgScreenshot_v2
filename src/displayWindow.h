#ifndef DISWIN_h
#define DISWIN_h

#include <windows.h>

#include <chrono>
#include <thread>
#include <utility>


std::pair<double, double> getScalingFactors();

LRESULT CALLBACK ScreenShotWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void displayBitmap();

// Captures the contents of the primary screen into the given bitmap.
void captureScreenToBitmap(HBITMAP *hBitmap);

// handle to the bitmap that stores the screenshot. Defined in
// displayWindow.cpp so that it can be used by multiple translation units.
extern HBITMAP screenBitmap;

// Copies the selected area of the screen bitmap to the clipboard.
void copySelectionToClipboard(POINT start, POINT end);

// Saves the given bitmap as a PNG file. Uses OpenCV to detect nearly white
// background pixels and make them transparent. Returns true on success.
bool saveBitmapAsPngWithoutBackground(HBITMAP bitmap, const wchar_t* filename);

#endif  // DISWIN_h
