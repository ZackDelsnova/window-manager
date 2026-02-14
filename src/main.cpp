#include <windows.h>
#include <dwmapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

struct WindowInfo {
    HWND hwnd;
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
