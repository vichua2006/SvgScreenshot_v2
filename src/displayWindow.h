#ifndef DISWIN_h
#define DISWIN_h

#include <Windows.h>

#include <chrono>
#include <thread>


std::pair<double, double> getScalingFactors();

LRESULT CALLBACK ScreenShotWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void displayBitmap();

void captureScreenToBitmap(HBITMAP *hBitmap) {
#endif