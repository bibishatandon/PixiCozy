# ❀ PixiCozy — A Cozy Pastel Pixel Art Workspace

PixiCozy is a lightweight, visually charming, and cozy pastel pixel art editor built from scratch in C++ and SDL2. The application features a dedicated Welcome Screen, a dual-layer system, custom-designed pastel panels, anti-aliased font rendering via SDL_ttf, an undo/redo manager, and PNG exports.

It is structured to be clean, modular, and easy to build, making it a strong asset for portfolio showcases and academic project evaluations.

---

## 1. Directory Structure

Here is what every folder and file in the project does:

```
PixiCozy/
├── CMakeLists.txt                 # CMake build configuration and dependency mappings
├── README.md                      # This documentation guide
├── pixicozy.ini                   # Local config file tracking up to 3 recent files
├── SDL2.dll                       # 32-bit runtime dynamic library for SDL
├── SDL2_ttf.dll                   # 32-bit runtime dynamic library for SDL_ttf
├── .vscode/
│   ├── tasks.json                 # Automates CMake configure and build tasks in VS Code
│   └── launch.json                # Configures the VS Code debugger to launch the app
├── assets/
│   └── fonts/
│       └── Nunito-Bold.ttf        # Cozy, rounded font loaded by SDL_ttf
├── scripts/
│   └── setup_dependencies.ps1     # Automation script to fetch libraries and font files
├── thirdparty/                    # Local header and library files
│   ├── SDL2/                      # Core windowing and rendering libraries
│   ├── SDL2_ttf/                  # Anti-aliased font rendering headers and libs
│   └── stb/                       # Single-header PNG writing library
└── src/
    ├── main.cpp                   # Application entry, frame controller, and event loop
    ├── canvas.h                   # Header for the Canvas model
    ├── canvas.cpp                 # Pixel drawing, coordinate checks, and PNG export routines
    ├── layer.h                    # 2D layer pixel structure and visibility tokens
    ├── history.h                  # Header for the Undo/Redo snapshot system
    ├── history.cpp                # History state restoration logic
    ├── ui.h                       # Header for UI elements and design tokens
    └── ui.cpp                     # Custom button widgets and vector icon drawing
```

---

## 2. Architecture & Design Patterns

The application is built on a modular C++ architecture separating visual drawing routines from data structures:

1. **Model (Canvas & Layers):**
   * **`layer.h`** stores the pixels of a single layer as a 2D grid (`std::vector<std::vector<SDL_Color>>`).
   * **`canvas.h/cpp`** manages Layer 1 and Layer 2. It composites the two layers using an **Alpha Blending** algorithm to produce the visual image. It also implements operations like **Flood Fill** (using a queue-based Breadth-First-Search) and file saving.
2. **State Management (History):**
   * **`history.h/cpp`** captures deep copies of the layers when an edit starts. By swapping entire pixel grids, it avoids complex undo pointers and remains stable.
3. **Renderer & Layouts (UI):**
   * **`ui.h/cpp`** houses the styling coordinates and theme colors. Instead of using generic OS designs, panels are custom-drawn using raw SDL rectangle commands with trimmed corner pixels to give a handmade, rounded, pixel-art look. Icons like the flower and eye visibility markers are rendered as pixel vectors.
4. **Controller (Main Loop):**
   * **`main.cpp`** binds everything. It runs the window loop capped at ~60 FPS, captures keyboard/mouse events, checks button mouse overlaps, and routes drawing vectors to the Canvas.

---

## 3. Setup and Build Guide

### A. Pre-requisites
1. **MinGW GCC Compiler:** Ensure you have `g++` installed. You can check this by typing `g++ --version` in your terminal.
2. **Python & Pip:** Python is used to install CMake. If `pip` is available, run the following command to install CMake:
   ```powershell
   pip install cmake
   ```

### B. Setup Dependencies
Open a PowerShell terminal in the workspace root and run the setup script:
```powershell
powershell -ExecutionPolicy Bypass -File scripts/setup_dependencies.ps1
```
This script downloads, extracts, and places the 32-bit SDL2, SDL2_ttf libraries, the Nunito-Bold font, and the `stb_image_write.h` header into the project folders, then copies the runtime DLLs to the root.

### C. Build in VS Code
1. Open the project folder in VS Code.
2. Run the build command: Press `Ctrl + Shift + B`. This triggers **CMake Configure** (creating build files) followed by **CMake Build** (compiling the code).
3. The executable `PixiCozy.exe` will be generated inside the `build/` folder.

### D. Run & Debug
* Press **F5** in VS Code to run the project with the debugger attached.
* Alternatively, run the executable directly in your terminal:
  ```powershell
  .\build\PixiCozy.exe
  ```

---

## 4. User Controls & Interface

### Welcome Screen
* **Create New Canvas:** Choose between three size presets: `16 x 16`, `32 x 32`, or `64 x 64`.
* **Load Custom Artwork:** Opens a native Windows Open Dialog to select a previously saved `.pxc` drawing.
* **Recent Files:** A list of the last 3 opened or saved files. Click any filename to load it immediately.

### Drawing Workspace
* **Left Click & Drag:** Paints on the active layer using the selected tool and color.
* **Right Click:** Picks the color of the pixel currently under the cursor (Pencil/Eraser active).
* **Mouse Scroll Wheel:** Zooms in and out of the canvas centered on the workspace.
* **Ctrl + Mouse Wheel:** Performs precise zooming.

### Controls Toolbar
* **Pencil:** Paints pixels in the active pastel color.
* **Eraser:** Clears pixels. On Layer 1 (background), it paints solid white. On Layer 2 (foreground), it paints transparent pixels.
* **Bucket Fill:** Contiguously fills pixels of identical color.
* **Eye Dropper:** Samples the blended color from the canvas.
* **Undo/Redo:** Backtrack or re-apply steps.
* **Clear Canvas:** Clears the active layer.
* **Save/Load:** Saves or loads the project in the custom human-readable `.pxc` text format.
* **Export to PNG:** Saves the blended image as a standard PNG file.
* **Back to Menu:** Returns to the Welcome Screen.

---

## 5. Potential Future Upgrades (Resume Value)

If you are looking to expand this project to showcase more advanced skills in interviews, consider the following ideas:
1. **Dynamic Custom Palettes:** Allow users to save their current color choices into custom palette configuration files.
2. **Dithering Brush:** Implement a dithered brush pattern (checkerboard blending) to create soft shadow gradients on the pixel canvas.
3. **Canvas Resizing:** Implement a crop tool that changes the canvas canvas width and height dynamically without wiping the active drawings.
4. **Custom Keybinds:** Create a key-mapping parser (`keybinds.ini`) that lets users customize keys for tools (e.g. `B` for bucket, `E` for eraser).
