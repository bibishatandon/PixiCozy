#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "canvas.h"
#include "history.h"
#include "ui.h"

// Win32 Native File Dialog integration
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>

std::string open_file_dialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "PixiCozy Art Files (*.pxc)\0*.pxc\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "pxc";
    
    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
    return "";
}

std::string save_file_dialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "PixiCozy Art Files (*.pxc)\0*.pxc\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "pxc";
    
    if (GetSaveFileNameA(&ofn)) {
        return std::string(filename);
    }
    return "";
}

std::string export_file_dialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "PNG Image Files (*.png)\0*.png\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "png";
    
    if (GetSaveFileNameA(&ofn)) {
        return std::string(filename);
    }
    return "";
}
#else
std::string open_file_dialog() { return "artwork.pxc"; }
std::string save_file_dialog() { return "artwork.pxc"; }
std::string export_file_dialog() { return "export.png"; }
#endif

// ==========================================
// ❀ CONFIG / RECENT FILES UTILITIES
// ==========================================
std::string get_base_name(const std::string& path) {
    size_t last_slash = path.find_last_of("\\/");
    if (last_slash == std::string::npos) return path;
    return path.substr(last_slash + 1);
}

std::vector<std::string> load_recent_files() {
    std::vector<std::string> files;
    std::ifstream file("pixicozy.ini");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("recent=", 0) == 0) {
                std::string path = line.substr(7);
                if (!path.empty()) {
                    files.push_back(path);
                }
            }
        }
        file.close();
    }
    return files;
}

void save_recent_files(const std::vector<std::string>& files) {
    std::ofstream file("pixicozy.ini");
    if (file.is_open()) {
        for (const auto& path : files) {
            file << "recent=" << path << "\n";
        }
        file.close();
    }
}

void add_recent_file(std::vector<std::string>& files, const std::string& path) {
    if (path.empty()) return;
    files.erase(std::remove(files.begin(), files.end(), path), files.end());
    files.insert(files.begin(), path);
    if (files.size() > 3) {
        files.resize(3);
    }
    save_recent_files(files);
}

// ==========================================
// ❀ APPLICATION ENUMS & STRUCTURES
// ==========================================
enum AppState {
    STATE_WELCOME,
    STATE_EDITOR
};

enum ToolType {
    TOOL_PENCIL,
    TOOL_ERASER,
    TOOL_BUCKET,
    TOOL_PICKER
};

int main(int argc, char* argv[]) {
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Window configuration
    const int WINDOW_WIDTH = 960;
    const int WINDOW_HEIGHT = 640;
    SDL_Window* window = SDL_CreateWindow(
        "PixiCozy - Cozy Pastel Pixel Art Workspace",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, 
        -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load Silkscreen pixel font at optimized integer sizes
    TTF_Font* fontTitle = TTF_OpenFont("assets/fonts/Silkscreen-Bold.ttf", 18);
    TTF_Font* fontNormal = TTF_OpenFont("assets/fonts/Silkscreen-Regular.ttf", 12);
    TTF_Font* fontSmall = TTF_OpenFont("assets/fonts/Silkscreen-Regular.ttf", 8);

    if (!fontTitle || !fontNormal || !fontSmall) {
        std::cerr << "Warning: Could not load Silkscreen font. TTF_Error: " << TTF_GetError() << std::endl;
    }

    // State Variables
    AppState appState = STATE_WELCOME;
    Canvas canvas(32, 32); // Initial placeholder size
    History history;
    ToolType activeTool = TOOL_PENCIL;
    SDL_Color activeColor = PRESET_PALETTE[7]; // Default accent color (Pink)
    int zoom = 12; // Visual pixel multiplier

    // Drag-Drawing tracking
    bool isPainting = false;
    bool hasPushedHistoryThisStroke = false;

    // Load recent files configuration
    std::vector<std::string> recentFiles = load_recent_files();

    // ==========================================
    // ❀ UI BUTTON BINDINGS & COORDINATES
    // ==========================================
    
    // Welcome Screen Buttons
    Button btnNewCanvas("New Canvas", 330, 240, 300, 40);
    Button btnLoadFile("Load Custom Artwork (.pxc)", 330, 340, 300, 40);
    Button btnExitApp("Exit Application", 330, 390, 300, 40);

    // New Canvas size selectors (hidden by default until New Canvas is clicked)
    bool showSizeSelect = false;
    Button btnSize16("16 x 16", 330, 290, 90, 30);
    Button btnSize32("32 x 32", 435, 290, 90, 30);
    Button btnSize64("64 x 64", 540, 290, 90, 30);

    // Editor Sidebar Layouts
    // Left Sidebar: x: 0 to 180
    Button btnToolPencil("Pencil", 15, 60, 150, 30);
    Button btnToolEraser("Eraser", 15, 95, 150, 30);
    Button btnToolBucket("Bucket Fill", 15, 130, 150, 30);
    Button btnToolPicker("Eye Dropper", 15, 165, 150, 30);

    Button btnEditUndo("Undo", 15, 240, 70, 30);
    Button btnEditRedo("Redo", 95, 240, 70, 30);
    Button btnEditClear("Clear Canvas", 15, 275, 150, 30);

    Button btnFileSave("Save", 15, 350, 70, 30);
    Button btnFileLoad("Load", 95, 350, 70, 30);
    Button btnFileExport("Export to PNG", 15, 385, 150, 30);

    Button btnBackToMenu("Back to Menu", 15, 545, 150, 35);

    // Zoom layout buttons (workspace bottom)
    Button btnZoomOut("-", 400, 560, 30, 30);
    Button btnZoomReset("100%", 440, 560, 80, 30);
    Button btnZoomIn("+", 530, 560, 30, 30);

    // Right Sidebar active layer selectors
    Button btnLayer2("Layer 2", 845, 50, 95, 30);
    Button btnLayer1("Layer 1", 845, 90, 95, 30);

    // Workspace margins
    const int WORKSPACE_X = 180;
    const int WORKSPACE_W = 600;
    const int WORKSPACE_H = 600;

    // Loop Control
    bool quit = false;
    SDL_Event event;

    // Mouse positions
    int mx = 0, my = 0;
    int canvasHoverX = -1;
    int canvasHoverY = -1;

    while (!quit) {
        // Event processing
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            // Capture mouse movements globally
            SDL_GetMouseState(&mx, &my);

            // 1. Mouse wheel zoom events (if editor state)
            if (appState == STATE_EDITOR && event.type == SDL_MOUSEWHEEL) {
                // Adjust zoom factor
                int oldZoom = zoom;
                if (event.wheel.y > 0) {
                    zoom = std::min(64, zoom + 2);
                } else if (event.wheel.y < 0) {
                    zoom = std::max(2, zoom - 2);
                }
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // ==========================================
                    // STATE: WELCOME INTERACTION
                    // ==========================================
                    if (appState == STATE_WELCOME) {
                        if (btnNewCanvas.checkHover(mx, my)) {
                            showSizeSelect = !showSizeSelect;
                        }
                        else if (showSizeSelect && btnSize16.checkHover(mx, my)) {
                            canvas.resize(16, 16);
                            zoom = 24; // Better default zoom for small sizes
                            history.clear();
                            appState = STATE_EDITOR;
                            showSizeSelect = false;
                        }
                        else if (showSizeSelect && btnSize32.checkHover(mx, my)) {
                            canvas.resize(32, 32);
                            zoom = 12;
                            history.clear();
                            appState = STATE_EDITOR;
                            showSizeSelect = false;
                        }
                        else if (showSizeSelect && btnSize64.checkHover(mx, my)) {
                            canvas.resize(64, 64);
                            zoom = 6;
                            history.clear();
                            appState = STATE_EDITOR;
                            showSizeSelect = false;
                        }
                        else if (btnLoadFile.checkHover(mx, my)) {
                            std::string path = open_file_dialog();
                            if (!path.empty() && canvas.loadFromFile(path)) {
                                add_recent_file(recentFiles, path);
                                history.clear();
                                // Set visual zoom levels based on file dimensions
                                if (canvas.getWidth() == 16) zoom = 24;
                                else if (canvas.getWidth() == 64) zoom = 6;
                                else zoom = 12;
                                appState = STATE_EDITOR;
                            }
                        }
                        else if (btnExitApp.checkHover(mx, my)) {
                            quit = true;
                        }
                        else {
                            // Check recent files clicks
                            for (size_t i = 0; i < recentFiles.size(); ++i) {
                                SDL_Rect slot = { 330, static_cast<int>(460 + i * 25), 300, 22 };
                                if (mx >= slot.x && mx < slot.x + slot.w && my >= slot.y && my < slot.y + slot.h) {
                                    std::string path = recentFiles[i];
                                    if (canvas.loadFromFile(path)) {
                                        add_recent_file(recentFiles, path);
                                        history.clear();
                                        if (canvas.getWidth() == 16) zoom = 24;
                                        else if (canvas.getWidth() == 64) zoom = 6;
                                        else zoom = 12;
                                        appState = STATE_EDITOR;
                                    }
                                }
                            }
                        }
                    }
                    // ==========================================
                    // STATE: EDITOR INTERACTION
                    // ==========================================
                    else if (appState == STATE_EDITOR) {
                        // Check tool selection clicks
                        if (btnToolPencil.checkHover(mx, my)) activeTool = TOOL_PENCIL;
                        else if (btnToolEraser.checkHover(mx, my)) activeTool = TOOL_ERASER;
                        else if (btnToolBucket.checkHover(mx, my)) activeTool = TOOL_BUCKET;
                        else if (btnToolPicker.checkHover(mx, my)) activeTool = TOOL_PICKER;

                        // Check editing actions
                        else if (btnEditUndo.checkHover(mx, my)) {
                            history.undo(canvas);
                        }
                        else if (btnEditRedo.checkHover(mx, my)) {
                            history.redo(canvas);
                        }
                        else if (btnEditClear.checkHover(mx, my)) {
                            history.pushState(canvas);
                            canvas.clearActiveLayer();
                        }

                        // Check File Action Clicks
                        else if (btnFileSave.checkHover(mx, my)) {
                            std::string path = save_file_dialog();
                            if (!path.empty()) {
                                if (canvas.saveToFile(path)) {
                                    add_recent_file(recentFiles, path);
                                }
                            }
                        }
                        else if (btnFileLoad.checkHover(mx, my)) {
                            std::string path = open_file_dialog();
                            if (!path.empty()) {
                                if (canvas.loadFromFile(path)) {
                                    add_recent_file(recentFiles, path);
                                    history.clear();
                                }
                            }
                        }
                        else if (btnFileExport.checkHover(mx, my)) {
                            std::string path = export_file_dialog();
                            if (!path.empty()) {
                                canvas.exportToPNG(path);
                            }
                        }

                        // Check Back to Menu
                        else if (btnBackToMenu.checkHover(mx, my)) {
                            appState = STATE_WELCOME;
                        }

                        // Check Zoom buttons
                        else if (btnZoomIn.checkHover(mx, my)) {
                            zoom = std::min(64, zoom + 2);
                        }
                        else if (btnZoomOut.checkHover(mx, my)) {
                            zoom = std::max(2, zoom - 2);
                        }
                        else if (btnZoomReset.checkHover(mx, my)) {
                            if (canvas.getWidth() == 16) zoom = 24;
                            else if (canvas.getWidth() == 64) zoom = 6;
                            else zoom = 12;
                        }

                        // Check layer selection clicks
                        else if (btnLayer1.checkHover(mx, my)) {
                            canvas.setActiveLayerIndex(0);
                        }
                        else if (btnLayer2.checkHover(mx, my)) {
                            canvas.setActiveLayerIndex(1);
                        }

                        // Check Layer visibility clicks
                        // Layer 2 Eye: x: 795, y: 50, scale: 2 (grid 7x5 -> 14x10 px)
                        // Make visibility click box a bit larger for comfort
                        else if (mx >= 795 && mx < 835 && my >= 45 && my < 85) {
                            canvas.getLayer(1).visible = !canvas.getLayer(1).visible;
                        }
                        // Layer 1 Eye: x: 795, y: 90
                        else if (mx >= 795 && mx < 835 && my >= 85 && my < 125) {
                            canvas.getLayer(0).visible = !canvas.getLayer(0).visible;
                        }

                        // Check Preset color swatch clicks
                        // Palette Swatch starts at x=800, y=200. Swatch size is 30px, gap is 8px.
                        for (int i = 0; i < 16; ++i) {
                            int rIdx = i / 4;
                            int cIdx = i % 4;
                            int sx = 800 + cIdx * 38;
                            int sy = 200 + rIdx * 38;
                            if (mx >= sx && mx < sx + 30 && my >= sy && my < sy + 30) {
                                activeColor = PRESET_PALETTE[i];
                            }
                        }

                        // Check if click is inside canvas area
                        // Canvas centering layout calculations
                        int canvas_w = canvas.getWidth() * zoom;
                        int canvas_h = canvas.getHeight() * zoom;
                        int canvas_x = WORKSPACE_X + (WORKSPACE_W - canvas_w) / 2;
                        int canvas_y = (WORKSPACE_H - canvas_h) / 2;

                        if (mx >= canvas_x && mx < canvas_x + canvas_w &&
                            my >= canvas_y && my < canvas_y + canvas_h) {
                            
                            int cx = (mx - canvas_x) / zoom;
                            int cy = (my - canvas_y) / zoom;

                            if (activeTool == TOOL_BUCKET) {
                                history.pushState(canvas);
                                canvas.floodFill(cx, cy, activeColor);
                            }
                            else if (activeTool == TOOL_PICKER) {
                                SDL_Color blended = canvas.getBlendedPixel(cx, cy);
                                if (blended.a > 0) {
                                    activeColor = blended;
                                }
                            }
                            else {
                                // Pencil or Eraser drag-draw initialization
                                isPainting = true;
                                hasPushedHistoryThisStroke = false;
                            }
                        }
                    }
                }
            }

            if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    isPainting = false;
                    hasPushedHistoryThisStroke = false;
                }
            }
        }

        // ==========================================
        // DRAG DRAWING STROKE INTERACTION (Pencil / Eraser)
        // ==========================================
        if (appState == STATE_EDITOR && isPainting) {
            int canvas_w = canvas.getWidth() * zoom;
            int canvas_h = canvas.getHeight() * zoom;
            int canvas_x = WORKSPACE_X + (WORKSPACE_W - canvas_w) / 2;
            int canvas_y = (WORKSPACE_H - canvas_h) / 2;

            if (mx >= canvas_x && mx < canvas_x + canvas_w &&
                my >= canvas_y && my < canvas_y + canvas_h) {

                int cx = (mx - canvas_x) / zoom;
                int cy = (my - canvas_y) / zoom;

                // Push history snapshot exactly once at the beginning of a stroke drag
                if (!hasPushedHistoryThisStroke) {
                    history.pushState(canvas);
                    hasPushedHistoryThisStroke = true;
                }

                if (activeTool == TOOL_PENCIL) {
                    canvas.setPixel(cx, cy, activeColor);
                }
                else if (activeTool == TOOL_ERASER) {
                    // Layer 1 defaults to solid white, Layer 2 defaults to transparent overlay
                    SDL_Color clearColor = (canvas.getActiveLayerIndex() == 0) ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 0};
                    canvas.setPixel(cx, cy, clearColor);
                }
            }
        }

        // ==========================================
        // RENDER SCREEN PASSE
        // ==========================================
        
        // Clear background
        SDL_SetRenderDrawColor(renderer, UITheme::Background.r, UITheme::Background.g, UITheme::Background.b, 255);
        SDL_RenderClear(renderer);

        // ==========================================
        // RENDER: WELCOME STATE RENDERPASSE
        // ==========================================
        if (appState == STATE_WELCOME) {
            // Draw central welcome container panel
            SDL_Rect welcomeRect = { 280, 100, 400, 440 };
            draw_pixel_panel(renderer, welcomeRect, UITheme::PanelBg, UITheme::PanelBorder, 3);

            // Draw Flower Logo ❀
            // Centered relative to the panel
            draw_pixel_flower(renderer, 465, 140, 6); // Scale 6 creates a 30x30 logo

            // Draw title text
            draw_text_centered(renderer, fontTitle, "PixiCozy", 480, 200, UITheme::Text);
            draw_text_centered(renderer, fontNormal, "A Cozy Pastel Pixel Art Workspace", 480, 220, UITheme::MutedText);

            // Setup hover highlights
            btnNewCanvas.checkHover(mx, my);
            btnLoadFile.checkHover(mx, my);
            btnExitApp.checkHover(mx, my);

            // Draw primary buttons
            draw_button(renderer, fontNormal, btnNewCanvas);
            
            // If New Canvas clicked, display dimensions selection swatches
            if (showSizeSelect) {
                btnSize16.checkHover(mx, my);
                btnSize32.checkHover(mx, my);
                btnSize64.checkHover(mx, my);
                draw_button(renderer, fontNormal, btnSize16);
                draw_button(renderer, fontNormal, btnSize32);
                draw_button(renderer, fontNormal, btnSize64);
            }

            draw_button(renderer, fontNormal, btnLoadFile);
            draw_button(renderer, fontNormal, btnExitApp);

            // Draw Recent Files Panel inside container
            draw_text(renderer, fontNormal, "Recent Files", 330, 440, UITheme::MutedText);
            for (size_t i = 0; i < recentFiles.size(); ++i) {
                SDL_Rect slot = { 330, static_cast<int>(460 + i * 25), 300, 22 };
                bool isSlotHovered = (mx >= slot.x && mx < slot.x + slot.w && my >= slot.y && my < slot.y + slot.h);
                
                SDL_Color slotBg = isSlotHovered ? UITheme::SecondaryAccent : UITheme::PanelBg;
                SDL_SetRenderDrawColor(renderer, slotBg.r, slotBg.g, slotBg.b, 255);
                SDL_RenderFillRect(renderer, &slot);

                // Draw filename in the slots
                std::string base = get_base_name(recentFiles[i]);
                draw_text(renderer, fontSmall, base, 340, slot.y + 3, UITheme::Text);
            }

            // Draw branding footer (No compiler references)
            draw_text_centered(renderer, fontSmall, "A Cozy Pixel Art Workspace", 480, 570, UITheme::MutedText);
        }
        // ==========================================
        // RENDER: EDITOR STATE RENDERPASSE
        // ==========================================
        else if (appState == STATE_EDITOR) {
            // Draw Workspace Background
            SDL_Rect wsRect = { WORKSPACE_X, 0, WORKSPACE_W, WORKSPACE_H };
            SDL_SetRenderDrawColor(renderer, UITheme::Background.r, UITheme::Background.g, UITheme::Background.b, 255);
            SDL_RenderFillRect(renderer, &wsRect);

            // Canvas centering coordinates
            int canvas_w = canvas.getWidth() * zoom;
            int canvas_h = canvas.getHeight() * zoom;
            int canvas_x = WORKSPACE_X + (WORKSPACE_W - canvas_w) / 2;
            int canvas_y = (WORKSPACE_H - canvas_h) / 2;

            // Draw main Canvas board background (in case of total transparency)
            SDL_Rect canvasRect = { canvas_x, canvas_y, canvas_w, canvas_h };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &canvasRect);

            // Render Pixels
            for (int y = 0; y < canvas.getHeight(); ++y) {
                for (int x = 0; x < canvas.getWidth(); ++x) {
                    SDL_Color blended = canvas.getBlendedPixel(x, y);
                    
                    // If pixel is semi-transparent, draw a soft checkerboard underneath
                    if (blended.a < 255) {
                        int checkerSize = std::max(2, zoom / 2);
                        int px = canvas_x + x * zoom;
                        int py = canvas_y + y * zoom;
                        
                        // Draw 2x2 grid representing pixel bounds
                        for (int cy = 0; cy < zoom; cy += checkerSize) {
                            for (int cx = 0; cx < zoom; cx += checkerSize) {
                                bool isLight = ((cx / checkerSize) + (cy / checkerSize) + x + y) % 2 == 0;
                                SDL_Color checkCol = isLight ? SDL_Color{250, 250, 250, 255} : SDL_Color{225, 225, 225, 255};
                                SDL_SetRenderDrawColor(renderer, checkCol.r, checkCol.g, checkCol.b, 255);
                                SDL_Rect checkerRect = { px + cx, py + cy, std::min(checkerSize, zoom - cx), std::min(checkerSize, zoom - cy) };
                                SDL_RenderFillRect(renderer, &checkerRect);
                            }
                        }
                    }

                    // Render the actual pixel color (blending with the checkerboard if alpha > 0)
                    if (blended.a > 0) {
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        SDL_SetRenderDrawColor(renderer, blended.r, blended.g, blended.b, blended.a);
                        SDL_Rect pRect = { canvas_x + x * zoom, canvas_y + y * zoom, zoom, zoom };
                        SDL_RenderFillRect(renderer, &pRect);
                    }
                }
            }

            // Draw visual grid separating pixel borders (only if zoom factor is high enough)
            if (zoom >= 6) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, UITheme::CanvasBorder.r, UITheme::CanvasBorder.g, UITheme::CanvasBorder.b, 60);
                
                // Vertical grid lines
                for (int i = 0; i <= canvas.getWidth(); ++i) {
                    SDL_RenderDrawLine(renderer, canvas_x + i * zoom, canvas_y, canvas_x + i * zoom, canvas_y + canvas_h);
                }
                // Horizontal grid lines
                for (int j = 0; j <= canvas.getHeight(); ++j) {
                    SDL_RenderDrawLine(renderer, canvas_x, canvas_y + j * zoom, canvas_x + canvas_w, canvas_y + j * zoom);
                }
            }

            // Draw outer canvas border frame
            SDL_SetRenderDrawColor(renderer, UITheme::CanvasBorder.r, UITheme::CanvasBorder.g, UITheme::CanvasBorder.b, 255);
            SDL_RenderDrawRect(renderer, &canvasRect);

            // ==========================================
            // RENDER: SIDEBAR PANELS
            // ==========================================
            
            // 1. LEFT SIDEBAR (Width: 180px)
            SDL_Rect sidebarLeft = { 0, 0, WORKSPACE_X, WORKSPACE_H };
            draw_pixel_panel(renderer, sidebarLeft, UITheme::PanelBg, UITheme::PanelBorder, 2);

            // Brand Header ❀ PixiCozy
            draw_pixel_flower(renderer, 15, 20, 3);
            draw_text(renderer, fontTitle, "PixiCozy", 36, 15, UITheme::Text);

            // Left Sidebar Sections
            draw_text(renderer, fontSmall, "TOOLS", 15, 45, UITheme::MutedText);

            btnToolPencil.isActive = (activeTool == TOOL_PENCIL);
            btnToolEraser.isActive = (activeTool == TOOL_ERASER);
            btnToolBucket.isActive = (activeTool == TOOL_BUCKET);
            btnToolPicker.isActive = (activeTool == TOOL_PICKER);

            btnToolPencil.checkHover(mx, my);
            btnToolEraser.checkHover(mx, my);
            btnToolBucket.checkHover(mx, my);
            btnToolPicker.checkHover(mx, my);

            draw_button(renderer, fontNormal, btnToolPencil);
            draw_button(renderer, fontNormal, btnToolEraser);
            draw_button(renderer, fontNormal, btnToolBucket);
            draw_button(renderer, fontNormal, btnToolPicker);

            draw_text(renderer, fontSmall, "EDITING", 15, 225, UITheme::MutedText);
            btnEditUndo.checkHover(mx, my);
            btnEditRedo.checkHover(mx, my);
            btnEditClear.checkHover(mx, my);
            
            draw_button(renderer, fontNormal, btnEditUndo);
            draw_button(renderer, fontNormal, btnEditRedo);
            draw_button(renderer, fontNormal, btnEditClear);

            draw_text(renderer, fontSmall, "FILE", 15, 335, UITheme::MutedText);
            btnFileSave.checkHover(mx, my);
            btnFileLoad.checkHover(mx, my);
            btnFileExport.checkHover(mx, my);
            
            draw_button(renderer, fontNormal, btnFileSave);
            draw_button(renderer, fontNormal, btnFileLoad);
            draw_button(renderer, fontNormal, btnFileExport);

            btnBackToMenu.checkHover(mx, my);
            draw_button(renderer, fontNormal, btnBackToMenu);

            // 2. RIGHT SIDEBAR (Width: 180px, starting x=780)
            SDL_Rect sidebarRight = { 780, 0, 180, WORKSPACE_H };
            draw_pixel_panel(renderer, sidebarRight, UITheme::PanelBg, UITheme::PanelBorder, 2);

            // Right Sidebar Sections
            draw_text(renderer, fontSmall, "LAYERS", 795, 15, UITheme::MutedText);

            // Layer Visibility Eyes (O/V represented as text placeholder or vector eye glyph)
            // Layer 2 visibility eye at (795, 52)
            draw_pixel_eye(renderer, 795, 54, canvas.getLayer(1).visible, 3);
            
            // Layer 1 visibility eye at (795, 92)
            draw_pixel_eye(renderer, 795, 94, canvas.getLayer(0).visible, 3);

            btnLayer2.isActive = (canvas.getActiveLayerIndex() == 1);
            btnLayer1.isActive = (canvas.getActiveLayerIndex() == 0);

            btnLayer2.checkHover(mx, my);
            btnLayer1.checkHover(mx, my);

            draw_button(renderer, fontNormal, btnLayer2);
            draw_button(renderer, fontNormal, btnLayer1);

            // Display active asterisk next to layer button
            if (canvas.getActiveLayerIndex() == 1) {
                draw_text(renderer, fontNormal, "*", 944, 52, UITheme::Text);
            } else {
                draw_text(renderer, fontNormal, "*", 944, 92, UITheme::Text);
            }

            draw_text(renderer, fontSmall, "PALETTE", 795, 175, UITheme::MutedText);

            // Render 16 Preset Swatches
            for (int i = 0; i < 16; ++i) {
                int rIdx = i / 4;
                int cIdx = i % 4;
                int sx = 800 + cIdx * 38;
                int sy = 200 + rIdx * 38;
                
                SDL_Rect sRect = { sx, sy, 30, 30 };
                
                // Swatch Fill
                SDL_SetRenderDrawColor(renderer, PRESET_PALETTE[i].r, PRESET_PALETTE[i].g, PRESET_PALETTE[i].b, 255);
                SDL_RenderFillRect(renderer, &sRect);

                // Highlight selected color swatch with a dark border
                bool isSelected = canvas.areColorsEqual(PRESET_PALETTE[i], activeColor);
                bool isHovered = (mx >= sx && mx < sx + 30 && my >= sy && my < sy + 30);
                
                if (isSelected) {
                    SDL_SetRenderDrawColor(renderer, UITheme::Text.r, UITheme::Text.g, UITheme::Text.b, 255);
                    SDL_Rect outline = { sx - 2, sy - 2, 34, 34 };
                    SDL_RenderDrawRect(renderer, &outline);
                } else if (isHovered) {
                    SDL_SetRenderDrawColor(renderer, UITheme::SecondaryAccent.r, UITheme::SecondaryAccent.g, UITheme::SecondaryAccent.b, 255);
                    SDL_Rect outline = { sx - 1, sy - 1, 32, 32 };
                    SDL_RenderDrawRect(renderer, &outline);
                } else {
                    SDL_SetRenderDrawColor(renderer, UITheme::PanelBorder.r, UITheme::PanelBorder.g, UITheme::PanelBorder.b, 255);
                    SDL_RenderDrawRect(renderer, &sRect);
                }
            }

            // Current color preview subpanel
            draw_text(renderer, fontSmall, "CURRENT COLOR", 795, 370, UITheme::MutedText);

            SDL_Rect previewColRect = { 800, 395, 40, 40 };
            SDL_SetRenderDrawColor(renderer, activeColor.r, activeColor.g, activeColor.b, 255);
            SDL_RenderFillRect(renderer, &previewColRect);
            
            SDL_SetRenderDrawColor(renderer, UITheme::Text.r, UITheme::Text.g, UITheme::Text.b, 255);
            SDL_RenderDrawRect(renderer, &previewColRect);

            // Hex and RGB component texts
            draw_text(renderer, fontNormal, colorToHex(activeColor), 850, 395, UITheme::Text);
            
            std::string rgbStr = std::to_string(activeColor.r) + "," + std::to_string(activeColor.g) + "," + std::to_string(activeColor.b);
            draw_text(renderer, fontSmall, rgbStr, 850, 415, UITheme::MutedText);

            // Workspace Bottom camera zoom controls
            btnZoomIn.checkHover(mx, my);
            btnZoomOut.checkHover(mx, my);
            btnZoomReset.checkHover(mx, my);
            draw_button(renderer, fontNormal, btnZoomIn);
            draw_button(renderer, fontNormal, btnZoomReset);
            draw_button(renderer, fontNormal, btnZoomOut);

            // ==========================================
            // RENDER: BOTTOM STATUS BAR (Height: 40px)
            // ==========================================
            SDL_Rect statusBar = { 0, 600, WINDOW_WIDTH, 40 };
            draw_pixel_panel(renderer, statusBar, UITheme::PanelBg, UITheme::PanelBorder, 1);

            // Construct coordinate status text if inside workspace
            std::string statusText = "";
            
            int cellX = -1;
            int cellY = -1;
            if (mx >= canvas_x && mx < canvas_x + canvas_w &&
                my >= canvas_y && my < canvas_y + canvas_h) {
                cellX = (mx - canvas_x) / zoom;
                cellY = (my - canvas_y) / zoom;
            }

            std::string toolName = "Pencil";
            if (activeTool == TOOL_ERASER) toolName = "Eraser";
            else if (activeTool == TOOL_BUCKET) toolName = "Bucket Fill";
            else if (activeTool == TOOL_PICKER) toolName = "Eye Dropper";

            std::string activeLayerName = canvas.getLayer(canvas.getActiveLayerIndex()).name;

            std::string posStr = (cellX != -1) ? ("(" + std::to_string(cellX) + "," + std::to_string(cellY) + ")") : "--";
            
            // Build visual pipeline string (Capitals/spaces formatted neatly)
            statusText = "Tool: " + toolName + " | Layer: " + activeLayerName + " | Color: " + colorToHex(activeColor) + " | Pos: " + posStr + " | Zoom: " + std::to_string(zoom * 100 / 12) + "%";
            
            draw_text(renderer, fontNormal, statusText, 20, 610, UITheme::Text);
        }

        // Present Frame to window buffer
        SDL_RenderPresent(renderer);
        
        // Small delay to cap framerate ~60FPS
        SDL_Delay(16);
    }

    // ==========================================
    // CLEANUP & RELEASES
    // ==========================================
    if (fontTitle) TTF_CloseFont(fontTitle);
    if (fontNormal) TTF_CloseFont(fontNormal);
    if (fontSmall) TTF_CloseFont(fontSmall);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
