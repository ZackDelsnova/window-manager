#include <windows.h>
#include <dwmapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

enum class LayoutState {
    LeftTop,
    RightTop,
    LeftBottom,
    RightBottom,
    Center
};

struct WindowInfo {
    HWND hwnd;
    LayoutState state;
};

std::vector<WindowInfo> managedWindows;

auto FindManagedWindow(HWND hwnd) {
    return std::find_if(
        managedWindows.begin(),
        managedWindows.end(),
        [hwnd](const WindowInfo& w) {
            return w.hwnd == hwnd;
        }
    );
}

void ApplyRightTop(HWND hwnd, int screenW, int screenH) {
    SetWindowPos(
        hwnd, nullptr,
        screenW / 2, 0,
        screenW / 2, screenH / 2,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

void ApplyRightBottom(HWND hwnd, int screenW, int screenH) {
    SetWindowPos(
        hwnd, nullptr,
        screenW / 2, screenH / 2,
        screenW / 2, screenH / 2,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

void ApplyLeftTop(HWND hwnd, int screenW, int screenH) {
    SetWindowPos(
        hwnd, nullptr,
        0, 0,
        screenW / 2, screenH / 2,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

void ApplyLeftBottom(HWND hwnd, int screenW, int screenH) {
    SetWindowPos(
        hwnd, nullptr,
        0, screenH / 2,
        screenW / 2, screenH / 2,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

void ApplyCenter(HWND hwnd, int screenW, int screenH) {
    int w = screenW * 0.6;
    int h = screenH * 0.8;
    int x = (screenW - w) / 2;
    int y = (screenH - h) / 2;
    SetWindowPos(
        hwnd, nullptr,
        x, y,
        w, h,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

void ApplyLayout(HWND hwnd, LayoutState state) {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    switch (state) {
        case LayoutState::LeftTop: {
            ApplyLeftTop(hwnd, screenW, screenH);
        } break;

        case LayoutState::RightTop: {
            ApplyRightTop(hwnd, screenW, screenH);
        } break;

        case LayoutState::LeftBottom: {
            ApplyLeftBottom(hwnd, screenW, screenH);
        } break;

        case LayoutState::RightBottom: {
            ApplyRightBottom(hwnd, screenW, screenH);
        } break;

        case LayoutState::Center: {
            ApplyCenter(hwnd, screenW, screenH);
        } break;
    }
}

bool IsLayoutOccupied(LayoutState state, HWND exclude = nullptr) {
    for (const auto& w : managedWindows) {
        if (w.state == state && w.hwnd != exclude) return true;
    }
    return false;
}

LayoutState GetNextFreeSlot(LayoutState start, HWND exclude = nullptr) {
    int startIndex = static_cast<int>(start);
    for (int i = 0; i < 5; ++i) {
        LayoutState candidate = static_cast<LayoutState>((startIndex + i) % 5);

        if (!IsLayoutOccupied(candidate, exclude)) return candidate;
    }

    return start;
}

bool IsManageableWindow(HWND hwnd) {
    if (!IsWindowVisible(hwnd)) return false;

    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    // resizable
    if (!(style & WS_THICKFRAME)) return false;

    // minimize and maximize
    if (!(style & WS_MINIMIZEBOX)) return false;
    if (!(style & WS_MAXIMIZEBOX)) return false;

    // ignore tool window and transparent overlays
    if (exStyle & WS_EX_TOOLWINDOW) return false;
    if (exStyle & WS_EX_TRANSPARENT) return false;

    // cloaked window popus and stuff
    BOOL cloaked = FALSE;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked) return false;

    return true;
}

void ApplyGridLayout()
{
    int count = managedWindows.size();
    if (count == 0) return;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    int cols = (int)std::ceil(std::sqrt(count));
    int rows = (int)std::ceil((float)count / cols);

    int cellW = screenW / cols;
    int cellH = screenH / rows;

    for (int i = 0; i < count; ++i)
    {
        int row = i / cols;
        int col = i % cols;

        int x = col * cellW;
        int y = row * cellH;

        SetWindowPos(
            managedWindows[i].hwnd,
            nullptr,
            x, y,
            cellW, cellH,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    }
}

void ApplyAdaptiveLayout() {
    int count = managedWindows.size();
    if (count == 0) return;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    int rows = 1;
    int cols = 1;
    
    if (count == 1)
    {
        rows = 1; cols = 1;
    }
    else if (count == 2)
    {
        rows = 1; cols = 2;
    }
    else if (count == 3)
    {
        rows = 3; cols = 1;
    }
    else if (count == 4)
    {
        rows = 2; cols = 2;
    }
    else
    {
        cols = (int)std::ceil(std::sqrt(count));
        rows = (int)std::ceil((float)count / cols);
    }

    int cellW = screenW / cols;
    int cellH = screenH / rows;

    for (int i = 0; i < count; ++i) {
        int row = i / cols;
        int col = i % cols;

        int x = col * cellW;
        int y = row * cellW;

        SetWindowPos(
            managedWindows[i].hwnd,
            nullptr,
            x, y,
            cellW, cellH,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    }
}

void ApplyMasterStackLayout() {
    int count = managedWindows.size();
    if (count == 0) return;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    for (auto& w : managedWindows) {
        ShowWindow(w.hwnd, SW_RESTORE);
    }

    if (count == 1) {
        SetWindowPos(
            managedWindows[0].hwnd,
            nullptr,
            0, 0,
            screenW, screenH,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
        return;
    }

    int masterWidth = screenW * 0.55;
    int stackWidth = screenW - masterWidth;

    // master
    SetWindowPos(
        managedWindows[0].hwnd,
        nullptr,
        0, 0,
        masterWidth, screenH,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    // stacks
    int stackCount = count - 1;
    int stackHeight = screenH / stackCount;

    for (int i = 1; i < count; i++) {
        int y = (i - 1) * stackHeight;
        int height = stackHeight;

        if (i == count - 1) {
            height = screenH - y;
        }

        SetWindowPos(
            managedWindows[i].hwnd,
            nullptr,
            masterWidth, y,
            stackWidth, height,
            SWP_NOZORDER | SWP_NOACTIVATE
        );
    }
}

int main() {
    while (true) {

        HWND active = GetForegroundWindow();
        HWND root = GetAncestor(active, GA_ROOTOWNER);

        if (IsManageableWindow(root)) {
            
            if (GetAsyncKeyState(VK_ESCAPE) & 1) {
                break;
            }

            managedWindows.erase(
                std::remove_if(
                    managedWindows.begin(),
                    managedWindows.end(),
                    [](const WindowInfo& w)
                    {
                        return !IsWindow(w.hwnd);
                    }),
                managedWindows.end()
            );

            if (GetAsyncKeyState(VK_F6) & 1) {
                auto it = FindManagedWindow(root);

                if (it == managedWindows.end()) {
                    WindowInfo info{};
                    info.hwnd = root;
                    managedWindows.push_back(info);
                }

                ApplyMasterStackLayout();
            }

            if (GetAsyncKeyState(VK_F7) & 1) {
                auto it = FindManagedWindow(root);
                if (it != managedWindows.end()) {
                    managedWindows.erase(it);
                }

                ApplyMasterStackLayout();
            }

        }   

        Sleep(10);
    }

    return 0;
}
