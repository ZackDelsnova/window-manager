
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
    std::string title;
    RECT rect;
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

int main() {
    while (true) {

        HWND active = GetForegroundWindow();
        HWND root = GetAncestor(active, GA_ROOTOWNER);

        if (root && IsWindowVisible(root)) {
            
            if (GetAsyncKeyState(VK_ESCAPE) & 1) {
                break;
            }

            if (GetAsyncKeyState(VK_F6) & 1) {
                auto it = FindManagedWindow(root);

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

                if (it == managedWindows.end()) {
                    LayoutState freeSlot = GetNextFreeSlot(LayoutState::LeftTop);

                    WindowInfo info{};
                    info.hwnd = root;
                    info.state = freeSlot;

                    managedWindows.push_back(info);
                } else {
                    LayoutState next = static_cast<LayoutState>((static_cast<int>(it->state) + 1) % 5);
                    next = GetNextFreeSlot(next, root);
                    it->state = next;
                }

                for (auto& w : managedWindows) {
                    ApplyLayout(w.hwnd, w.state);
                }

            }

        }   

        Sleep(10);
    }

    return 0;
}
