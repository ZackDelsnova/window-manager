
#include <windows.h>
#include <dwmapi.h>
#include <iostream>
#include <string>
#include <vector>

struct WindowInfo {
    HWND hwnd;
    std::string title;
    RECT rect;
    LONG_PTR style;
    LONG_PTR exStyle;
};

void PrintStyleFlags(LONG_PTR style) {
    if (style & WS_VISIBLE) std::cout << "WS_VISIBLE ";
    if (style & WS_CHILD) std::cout << "WS_CHILD ";
    if (style & WS_POPUP) std::cout << "WS_POPUP ";
    if (style & WS_CAPTION) std::cout << "WS_CAPTION ";
    if (style & WS_THICKFRAME) std::cout << "WS_THICKFRAME ";
    if (style & WS_MINIMIZEBOX) std::cout << "WS_MINIMIZEBOX ";
    if (style & WS_MAXIMIZEBOX) std::cout << "WS_MAXIMIZEBOX ";
    if (style & WS_OVERLAPPED) std::cout << "WS_OVERLAPPED ";
    if (style & WS_SYSMENU) std::cout << "WS_SYSMENU ";
    if (style & WS_DISABLED) std::cout << "WS_DISABLED ";
    if (style & WS_CLIPSIBLINGS) std::cout << "WS_CLIPSIBLINGS ";
    if (style & WS_CLIPCHILDREN) std::cout << "WS_CLIPCHILDREN ";
}

void PrintExStyleFlags(LONG_PTR exStyle) {
    if (exStyle & WS_EX_APPWINDOW) std::cout << "WS_EX_APPWINDOW ";
    if (exStyle & WS_EX_TOOLWINDOW) std::cout << "WS_EX_TOOLWINDOW ";
    if (exStyle & WS_EX_TOPMOST) std::cout << "WS_EX_TOPMOST ";
    if (exStyle & WS_EX_LAYERED) std::cout << "WS_EX_LAYERED ";
    if (exStyle & WS_EX_TRANSPARENT) std::cout << "WS_EX_TRANSPARENT ";
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam) {
    auto* windows = reinterpret_cast<std::vector<WindowInfo>*>(lparam);

    if (!IsWindowVisible(hwnd)) return TRUE;

    if (GetWindow(hwnd, GW_OWNER) != NULL) return TRUE;

    RECT r;
    if (!GetWindowRect(hwnd, &r)) return TRUE;

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));

    if (title[0] == '\0') return TRUE;

    WindowInfo info;
    info.hwnd = hwnd;
    info.title = title;
    info.rect = r;
    info.style = GetWindowLongPtr(hwnd, GWL_STYLE);
    info.exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    windows->push_back(info);

    return TRUE;
}

int main() {
    while (true) {
        system("cls");

        std::vector<WindowInfo> windows;
        EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));

        HWND active = GetForegroundWindow();

        for (const auto& w : windows) {
            if (w.hwnd == active) {
                std::cout << "Active Window:\n";
                std::cout << "Title: " << w.title << "\n";
                std::cout << "HWND: " << w.hwnd << "\n";
                std::cout << "x=" << w.rect.left
                          << " y=" << w.rect.top
                          << " w=" << (w.rect.right - w.rect.left)
                          << " h=" << (w.rect.bottom - w.rect.top)
                          << "\n\n";
                break;
            }
        }

        Sleep(500);
    }

    return 0;
}
