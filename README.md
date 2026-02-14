# Window Manager

hyprland like window manager in windows using win32 api.

## Requirements

1. MSVC compiler from Visual Studio
2. CMake 3.21
3. Ninja

## Building

1. build folder
```bash
cmake -S . -B build -G Ninja
```

2. Build
```bash
cmake --build build
```

3. Run
```bash
.\build\winwm.exe
```

## Controls

- F6 - Add current window to manager
- F7 - Remove current window from manager
- F8 - Increase Master window size
- F9 - Decrease Master window size
- F10 - Chnage current window to be master
- ESC - Quit

## Demo
<video width="320" height="240" controls>
    <source src="resources/windowmanager-demo.mp4" type="video/mp4">
    Check the video in the resources folder
</video>
