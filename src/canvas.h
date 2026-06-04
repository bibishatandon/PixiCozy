#ifndef CANVAS_H
#define CANVAS_H

#include <SDL.h>
#include <vector>
#include <string>
#include "layer.h"

class Canvas {
public:
    // Initializes the canvas with specified dimensions.
    Canvas(int width, int height);

    // Resizes the canvas to new dimensions, clearing all content.
    void resize(int width, int height);

    // Clears the active layer to its default state.
    void clearActiveLayer();

    // Clears both layers to their default state.
    void clearAll();

    // Set pixel color on the active layer at (x, y).
    void setPixel(int x, int y, SDL_Color color);

    // Gets the pixel color from a specific layer at (x, y).
    SDL_Color getPixel(int layerIdx, int x, int y) const;

    // Gets the visually blended pixel color at (x, y) taking visibility into account.
    SDL_Color getBlendedPixel(int x, int y) const;

    // Fills a contiguous region of identical colors on the active layer with targetColor.
    void floodFill(int startX, int startY, SDL_Color targetColor);

    // Saves the canvas data (all layers) to our custom text format (.pxc).
    bool saveToFile(const std::string& filepath);

    // Loads the canvas data from our custom format (.pxc).
    bool loadFromFile(const std::string& filepath);

    // Exports the blended visible canvas layers to a PNG file.
    bool exportToPNG(const std::string& filepath);

    // Getters and Setters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    int getActiveLayerIndex() const { return activeLayerIndex; }
    void setActiveLayerIndex(int index);

    Layer& getLayer(int index) { return layers[index]; }
    const Layer& getLayer(int index) const { return layers[index]; }

    // Helper to check if two colors are identical.
    bool areColorsEqual(SDL_Color c1, SDL_Color c2) const;

private:
    int width;
    int height;
    int activeLayerIndex;
    
    // We strictly support exactly 2 layers as frozen in scope.
    std::vector<Layer> layers;

    // Helper to check if coordinates are within the canvas grid bounds.
    bool isOutOfBounds(int x, int y) const;
};

#endif // CANVAS_H
