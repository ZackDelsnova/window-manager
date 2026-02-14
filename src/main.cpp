
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
    // LONG_PTR style;
    // LONG_PTR exStyle;
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

/*
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

void PrintHierarchy(HWND hwnd) {
    HWND owner = GetWindow(hwnd, GW_OWNER);
    HWND root = GetAncestor(hwnd, GA_ROOT);
    HWND rootOwner = GetAncestor(hwnd, GA_ROOTOWNER);
    std::cout << "Active HWND: " << hwnd << "\n";
    std::cout << "Owner: " << owner << "\n";
    std::cout << "Root: " << root << "\n";
    std::cout << "RootOwner: " << rootOwner << "\n";
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
*/
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

    LayoutState currentState = LayoutState::LeftTop;

    while (true) {
        system("cls");

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

            /*
            for (const auto& w : windows) {
                if (w.hwnd == root) {
                    std::cout << "Active Window:\n";
                    std::cout << "Title: " << w.title << "\n";
                    std::cout << "HWND: " << w.hwnd << "\n";
                    std::cout << "x=" << w.rect.left
                            << " y=" << w.rect.top
                            << " w=" << (w.rect.right - w.rect.left)
                            << " h=" << (w.rect.bottom - w.rect.top)
                            << "\n\n";
                    PrintStyleFlags(w.style);
                    PrintExStyleFlags(w.exStyle);
                    break;
                }
            }
            */
        }   

        Sleep(10);
    }

    return 0;
}
