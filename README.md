# ❀ PixiCozy

PixiCozy is a desktop pixel art editor built in C++ using SDL2 and SDL_ttf. It provides a focused workspace for creating pixel art with layer-based editing, undo/redo support, zoom controls, project saving, and PNG export.

The project was developed to explore desktop application development, graphics programming, and user interface design while building a complete application from the ground up.

## Features

- Pencil, Eraser, Bucket Fill, and Color Picker tools
- Two-layer editing system with visibility controls
- Undo and Redo support
- Mouse-wheel zoom and zoom buttons
- Save and load projects using a custom `.pxc` format
- Export artwork as PNG
- Recent files support on the welcome screen
- Custom pastel-themed interface rendered with SDL2

## Technology Stack

- C++
- SDL2
- SDL_ttf
- stb_image_write
- CMake

## Build and Run

### Requirements

- MinGW GCC
- Python with pip
- CMake

Install CMake:

```bash
pip install cmake
```

Set up project dependencies:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/setup_dependencies.ps1
```

Configure the project:

```bash
cmake -G "MinGW Makefiles" -B build
```

Build the project:

```bash
cmake --build build
```

Run PixiCozy:

```bash
.\build\PixiCozy.exe
```

## Controls

- Left click and drag: draw
- Right click: pick color
- Mouse wheel: zoom
- Ctrl + mouse wheel: fine zoom
- Ctrl + Z: undo
- Ctrl + Y: redo

```

## Notes

PixiCozy is intentionally lightweight and focused. The goal was to create a polished pixel art editor while gaining hands-on experience with graphics rendering, event-driven programming, custom UI development, and application architecture in C++.