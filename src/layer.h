#ifndef LAYER_H
#define LAYER_H

#include <SDL.h>
#include <vector>
#include <string>

// Represents a single canvas layer.
struct Layer {
    std::string name;
    bool visible = true;
    
    // Pixel storage formatted as pixels[row][col] or [y][x]
    std::vector<std::vector<SDL_Color>> pixels;

    Layer() = default;

    // Constructs a layer with specified size and fills it with a default color.
    Layer(const std::string& layerName, int width, int height, SDL_Color defaultColor) {
        name = layerName;
        visible = true;
        pixels.resize(height, std::vector<SDL_Color>(width, defaultColor));
    }
};

#endif // LAYER_H
