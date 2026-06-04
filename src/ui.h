#ifndef UI_H
#define UI_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

// ==========================================
// ❀ PIXICOZY COZY PASTEL DESIGN SYSTEM TOKENS
// ==========================================
namespace UITheme {
    const SDL_Color Background = {255, 247, 251, 255};      // #FFF7FB
    const SDL_Color PanelBg = {255, 228, 236, 255};         // #FFE4EC
    const SDL_Color PanelBorder = {247, 191, 212, 255};     // #F7BFD4
    const SDL_Color PrimaryAccent = {255, 183, 213, 255};   // #FFB7D5
    const SDL_Color SecondaryAccent = {220, 198, 255, 255}; // #DCC6FF
    const SDL_Color MintAccent = {200, 247, 220, 255};      // #C8F7DC
    const SDL_Color Text = {91, 75, 86, 255};               // #5B4B56
    const SDL_Color MutedText = {122, 104, 114, 255};       // #7A6872
    const SDL_Color CanvasBorder = {215, 184, 200, 255};    // #D7B8C8
}

// Preset color palette for swatches (16 Cozy Pastels)
extern const std::vector<SDL_Color> PRESET_PALETTE;

// ==========================================
// ❀ UI WIDGET STRUCTURES
// ==========================================
struct Button {
    std::string label;
    SDL_Rect rect;
    bool isHovered = false;
    bool isPressed = false;
    bool isActive = false; // For toggleable/selected buttons

    Button() = default;
    Button(const std::string& txt, int x, int y, int w, int h)
        : label(txt), rect({x, y, w, h}) {}

    // Updates hover state based on mouse coordinates. Returns true if hovered.
    bool checkHover(int mx, int my);
};

// ==========================================
// ❀ CUSTOM DRAWING AND RENDERING HELPERS
// ==========================================

// Draws a pixel-art style panel with truncated corner pixels for a cute handmade look.
void draw_pixel_panel(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color bg, SDL_Color border, int thickness = 2);

// Draws a custom 5x5 pixel-art flower glyph (❀ Logo) at (x, y).
void draw_pixel_flower(SDL_Renderer* renderer, int x, int y, int scale = 3);

// Draws a custom 7x5 pixel-art eye visibility glyph.
void draw_pixel_eye(SDL_Renderer* renderer, int x, int y, bool visible, int scale = 2);

// Standard text rendering wrapper.
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color);

// Centered text rendering wrapper.
void draw_text_centered(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int centerX, int centerY, SDL_Color color);

// Draws a button matching the PixiCozy styling (hover, pressed, active highlights).
void draw_button(SDL_Renderer* renderer, TTF_Font* font, const Button& btn);

// Utility to convert color components to Hexadecimal string.
std::string colorToHex(SDL_Color color);

#endif // UI_H
