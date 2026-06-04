#include "ui.h"
#include <iomanip>
#include <sstream>

// ==========================================
// ❀ DEFINITION OF THE 16 COZY PASTEL SWATCHES
// ==========================================
const std::vector<SDL_Color> PRESET_PALETTE = {
    SDL_Color{255, 183, 178, 255}, // Soft Red/Coral
    SDL_Color{255, 218, 193, 255}, // Soft Peach
    SDL_Color{226, 240, 203, 255}, // Creamy Yellow
    SDL_Color{181, 234, 215, 255}, // Mint Green
    SDL_Color{199, 206, 234, 255}, // Pastel Cyan
    SDL_Color{232, 197, 229, 255}, // Soft Lavender
    SDL_Color{216, 167, 177, 255}, // Dusty Rose
    SDL_Color{255, 183, 213, 255}, // Primary Accent Pink
    SDL_Color{220, 198, 255, 255}, // Secondary Lilac
    SDL_Color{200, 247, 220, 255}, // Pastel Sage
    SDL_Color{224, 169, 109, 255}, // Soft Terracotta
    SDL_Color{141, 110, 99, 255},  // Cozy Warm Brown
    SDL_Color{78, 88, 110, 255},   // Deep Slate Grey
    SDL_Color{46, 48, 51, 255},    // Charcoal (Off-Black)
    SDL_Color{255, 255, 255, 255}, // Pure White
    SDL_Color{236, 234, 228, 255}  // Cozy Sand (Warm Grey)
};

// ==========================================
// ❀ BUTTON METHODS
// ==========================================
bool Button::checkHover(int mx, int my) {
    isHovered = (mx >= rect.x && mx < rect.x + rect.w && 
                 my >= rect.y && my < rect.y + rect.h);
    return isHovered;
}

// ==========================================
// ❀ RENDERING FUNCTIONS
// ==========================================

void draw_pixel_panel(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color bg, SDL_Color border, int thickness) {
    // 1. Fill main rectangle body (excluding 1-pixel corners to simulate rounded retro design)
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    
    // Middle vertical block
    SDL_Rect middle = { rect.x + 1, rect.y, rect.w - 2, rect.h };
    SDL_RenderFillRect(renderer, &middle);
    
    // Left and right single-column edges (excluding top/bottom corner pixels)
    SDL_Rect leftCol = { rect.x, rect.y + 1, 1, rect.h - 2 };
    SDL_Rect rightCol = { rect.x + rect.w - 1, rect.y + 1, 1, rect.h - 2 };
    SDL_RenderFillRect(renderer, &leftCol);
    SDL_RenderFillRect(renderer, &rightCol);
    
    // 2. Draw border lines
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    
    // Top and Bottom lines (excluding corners)
    SDL_RenderDrawLine(renderer, rect.x + 1, rect.y, rect.x + rect.w - 2, rect.y);
    SDL_RenderDrawLine(renderer, rect.x + 1, rect.y + rect.h - 1, rect.x + rect.w - 2, rect.y + rect.h - 1);
    
    // Left and Right lines (excluding corners)
    SDL_RenderDrawLine(renderer, rect.x, rect.y + 1, rect.x, rect.y + rect.h - 2);
    SDL_RenderDrawLine(renderer, rect.x + rect.w - 1, rect.y + 1, rect.x + rect.w - 1, rect.y + rect.h - 2);
    
    // 3. Draw inner borders based on thickness
    for (int t = 1; t < thickness; ++t) {
        SDL_Rect r = { rect.x + t, rect.y + t, rect.w - 2 * t, rect.h - 2 * t };
        SDL_RenderDrawRect(renderer, &r);
    }
}

void draw_pixel_flower(SDL_Renderer* renderer, int x, int y, int scale) {
    // 5x5 Grid representation:
    // . P . P .
    // P P P P P
    // . P Y P .
    // P P P P P
    // . P . P .
    // (P = Primary Accent Pink, Y = Mint Accent Center, . = Transparent)
    SDL_Color pink = UITheme::PrimaryAccent;
    SDL_Color yellow = UITheme::MintAccent;

    auto draw_pixel = [&](int dx, int dy, SDL_Color c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_Rect r = { x + dx * scale, y + dy * scale, scale, scale };
        SDL_RenderFillRect(renderer, &r);
    };

    // Row 0
    draw_pixel(1, 0, pink); draw_pixel(3, 0, pink);
    // Row 1
    draw_pixel(0, 1, pink); draw_pixel(1, 1, pink); draw_pixel(2, 1, pink); draw_pixel(3, 1, pink); draw_pixel(4, 1, pink);
    // Row 2
    draw_pixel(1, 2, pink); draw_pixel(2, 2, yellow); draw_pixel(3, 2, pink);
    // Row 3
    draw_pixel(0, 3, pink); draw_pixel(1, 3, pink); draw_pixel(2, 3, pink); draw_pixel(3, 3, pink); draw_pixel(4, 3, pink);
    // Row 4
    draw_pixel(1, 4, pink); draw_pixel(3, 4, pink);
}

void draw_pixel_eye(SDL_Renderer* renderer, int x, int y, bool visible, int scale) {
    // 7x5 eye representation:
    // . . O O O . .
    // . O . . . O .
    // O . . X . . O
    // . O . . . O .
    // . . O O O . .
    // (O = Dark text contour, X = Light accent pupil)
    SDL_Color contour = UITheme::Text;
    SDL_Color pupil = UITheme::SecondaryAccent;
    SDL_Color hiddenSlash = UITheme::MutedText;

    auto draw_pixel = [&](int dx, int dy, SDL_Color c) {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_Rect r = { x + dx * scale, y + dy * scale, scale, scale };
        SDL_RenderFillRect(renderer, &r);
    };

    if (visible) {
        // Draw Contour
        draw_pixel(2, 0, contour); draw_pixel(3, 0, contour); draw_pixel(4, 0, contour);
        draw_pixel(1, 1, contour); draw_pixel(5, 1, contour);
        draw_pixel(0, 2, contour); draw_pixel(6, 2, contour);
        draw_pixel(1, 3, contour); draw_pixel(5, 3, contour);
        draw_pixel(2, 4, contour); draw_pixel(3, 4, contour); draw_pixel(4, 4, contour);

        // Draw Pupil
        draw_pixel(3, 2, pupil);
    } else {
        // Muted gray version representing closed/hidden eye
        SDL_Color mutedColor = UITheme::MutedText;
        draw_pixel(2, 0, mutedColor); draw_pixel(3, 0, mutedColor); draw_pixel(4, 0, mutedColor);
        draw_pixel(1, 1, mutedColor); draw_pixel(5, 1, mutedColor);
        draw_pixel(0, 2, mutedColor); draw_pixel(6, 2, mutedColor);
        draw_pixel(1, 3, mutedColor); draw_pixel(5, 3, mutedColor);
        draw_pixel(2, 4, mutedColor); draw_pixel(3, 4, mutedColor); draw_pixel(4, 4, mutedColor);
        
        // Draw slash cross line
        draw_pixel(1, 1, hiddenSlash);
        draw_pixel(2, 2, hiddenSlash);
        draw_pixel(3, 2, hiddenSlash);
        draw_pixel(4, 2, hiddenSlash);
        draw_pixel(5, 3, hiddenSlash);
    }
}

void draw_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty() || !font) return;
    
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dstRect = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }
    
    SDL_FreeSurface(surface);
}

void draw_text_centered(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int centerX, int centerY, SDL_Color color) {
    if (text.empty() || !font) return;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect dstRect = { centerX - surface->w / 2, centerY - surface->h / 2, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
    }

    SDL_FreeSurface(surface);
}

void draw_button(SDL_Renderer* renderer, TTF_Font* font, const Button& btn) {
    // Style selection based on current interactive states
    SDL_Color bg = SDL_Color{255, 255, 255, 255}; // Solid white base
    SDL_Color border = UITheme::PanelBorder;
    SDL_Color textCol = UITheme::Text;

    if (btn.isActive) {
        bg = UITheme::PrimaryAccent; // Cozy highlight for selected active option
        border = UITheme::Text;
    } else if (btn.isPressed) {
        bg = UITheme::PrimaryAccent;
        border = UITheme::PanelBorder;
    } else if (btn.isHovered) {
        bg = UITheme::SecondaryAccent; // Soft purple hover highlight
        border = UITheme::PanelBorder;
    }

    // Draw custom rounded button panel
    draw_pixel_panel(renderer, btn.rect, bg, border, 2);

    // Draw label
    draw_text_centered(
        renderer, 
        font, 
        btn.label, 
        btn.rect.x + btn.rect.w / 2, 
        btn.rect.y + btn.rect.h / 2, 
        textCol
    );
}

std::string colorToHex(SDL_Color color) {
    std::stringstream ss;
    ss << "#" 
       << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)color.r
       << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)color.g
       << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)color.b;
    return ss.str();
}
