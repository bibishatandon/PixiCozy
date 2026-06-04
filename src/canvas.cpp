#include "canvas.h"
#include <fstream>
#include <sstream>
#include <queue>

// Define STB image write implementation in this source file
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Canvas::Canvas(int width, int height) : width(0), height(0), activeLayerIndex(0) {
    resize(width, height);
}

void Canvas::resize(int w, int h) {
    width = w;
    height = h;
    
    // Default Layer 1: Solid opaque white canvas base
    SDL_Color layer1Default = {255, 255, 255, 255};
    // Default Layer 2: Fully transparent overlay
    SDL_Color layer2Default = {0, 0, 0, 0};

    layers.clear();
    layers.push_back(Layer("Layer 1", width, height, layer1Default));
    layers.push_back(Layer("Layer 2", width, height, layer2Default));
    
    activeLayerIndex = 0; // Default active layer is Layer 1
}

void Canvas::clearActiveLayer() {
    SDL_Color clearColor = (activeLayerIndex == 0) ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 0};
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            layers[activeLayerIndex].pixels[y][x] = clearColor;
        }
    }
}

void Canvas::clearAll() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            layers[0].pixels[y][x] = {255, 255, 255, 255};
            layers[1].pixels[y][x] = {0, 0, 0, 0};
        }
    }
}

void Canvas::setActiveLayerIndex(int index) {
    if (index >= 0 && index < 2) {
        activeLayerIndex = index;
    }
}

bool Canvas::isOutOfBounds(int x, int y) const {
    return (x < 0 || x >= width || y < 0 || y >= height);
}

bool Canvas::areColorsEqual(SDL_Color c1, SDL_Color c2) const {
    return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
}

void Canvas::setPixel(int x, int y, SDL_Color color) {
    if (isOutOfBounds(x, y)) return;
    layers[activeLayerIndex].pixels[y][x] = color;
}

SDL_Color Canvas::getPixel(int layerIdx, int x, int y) const {
    if (isOutOfBounds(x, y) || layerIdx < 0 || layerIdx >= 2) {
        return {0, 0, 0, 0};
    }
    return layers[layerIdx].pixels[y][x];
}

SDL_Color Canvas::getBlendedPixel(int x, int y) const {
    if (isOutOfBounds(x, y)) {
        return {0, 0, 0, 0};
    }

    // Blend Layer 2 (foreground) on top of Layer 1 (background)
    SDL_Color c1 = {255, 255, 255, 255};
    if (layers[0].visible) {
        c1 = layers[0].pixels[y][x];
    } else {
        // If Layer 1 is hidden, treat background as transparent
        c1 = {0, 0, 0, 0};
    }

    if (!layers[1].visible) {
        return c1;
    }

    SDL_Color c2 = layers[1].pixels[y][x];
    
    // Shortcut blends for common alpha cases
    if (c2.a == 255) return c2; // Foreground completely opaque
    if (c2.a == 0) return c1;   // Foreground completely transparent

    // standard alpha blending calculations
    float a2 = c2.a / 255.0f;
    float a1 = c1.a / 255.0f;
    
    float aOut = a2 + a1 * (1.0f - a2);
    if (aOut <= 0.0f) {
        return {0, 0, 0, 0};
    }

    Uint8 rOut = static_cast<Uint8>((c2.r * a2 + c1.r * a1 * (1.0f - a2)) / aOut);
    Uint8 gOut = static_cast<Uint8>((c2.g * a2 + c1.g * a1 * (1.0f - a2)) / aOut);
    Uint8 bOut = static_cast<Uint8>((c2.b * a2 + c1.b * a1 * (1.0f - a2)) / aOut);
    Uint8 alphaOut = static_cast<Uint8>(aOut * 255.0f);

    return {rOut, gOut, bOut, alphaOut};
}

void Canvas::floodFill(int startX, int startY, SDL_Color targetColor) {
    if (isOutOfBounds(startX, startY)) return;

    SDL_Color sourceColor = layers[activeLayerIndex].pixels[startY][startX];
    if (areColorsEqual(sourceColor, targetColor)) return; // Avoid infinite loops

    // BFS Queue-based flood fill to prevent stack overflow
    std::queue<std::pair<int, int>> pixelQueue;
    pixelQueue.push({startX, startY});
    layers[activeLayerIndex].pixels[startY][startX] = targetColor;

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    while (!pixelQueue.empty()) {
        auto current = pixelQueue.front();
        pixelQueue.pop();

        int cx = current.first;
        int cy = current.second;

        for (int i = 0; i < 4; ++i) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (!isOutOfBounds(nx, ny)) {
                SDL_Color neighborColor = layers[activeLayerIndex].pixels[ny][nx];
                if (areColorsEqual(neighborColor, sourceColor)) {
                    layers[activeLayerIndex].pixels[ny][nx] = targetColor;
                    pixelQueue.push({nx, ny});
                }
            }
        }
    }
}

bool Canvas::saveToFile(const std::string& filepath) {
    std::ofstream outFile(filepath);
    if (!outFile.is_open()) return false;

    // Header
    outFile << "VERSION 1\n";
    outFile << "SIZE " << width << " " << height << "\n";

    // Write Layer data
    for (int i = 0; i < 2; ++i) {
        outFile << "LAYER " << layers[i].name << " " << (layers[i].visible ? "1" : "0") << "\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                SDL_Color c = layers[i].pixels[y][x];
                outFile << (int)c.r << " " << (int)c.g << " " << (int)c.b << " " << (int)c.a << "\n";
            }
        }
    }

    outFile.close();
    return true;
}

bool Canvas::loadFromFile(const std::string& filepath) {
    std::ifstream inFile(filepath);
    if (!inFile.is_open()) return false;

    std::string line;
    
    // Read and verify Version
    if (!std::getline(inFile, line)) return false;
    if (line.find("VERSION 1") == std::string::npos) return false;

    // Read and verify Size
    if (!std::getline(inFile, line)) return false;
    std::stringstream sizeStream(line);
    std::string label;
    int loadedW = 0, loadedH = 0;
    sizeStream >> label >> loadedW >> loadedH;
    if (label != "SIZE" || loadedW <= 0 || loadedH <= 0) return false;

    // Allocate canvas space
    resize(loadedW, loadedH);

    // Read layers
    for (int i = 0; i < 2; ++i) {
        if (!std::getline(inFile, line)) return false;
        std::stringstream layerStream(line);
        std::string layerLabel, layerName;
        int isVisible = 1;
        layerStream >> layerLabel >> layerName;
        // Layer names can have spaces, but ours are "Layer" and "1" or "2"
        std::string suffix;
        layerStream >> suffix >> isVisible;
        layerName = layerName + " " + suffix;
        
        if (layerLabel != "LAYER") return false;
        layers[i].name = layerName;
        layers[i].visible = (isVisible == 1);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (!std::getline(inFile, line)) return false;
                std::stringstream pixelStream(line);
                int r, g, b, a;
                if (!(pixelStream >> r >> g >> b >> a)) return false;
                layers[i].pixels[y][x] = {
                    static_cast<Uint8>(r),
                    static_cast<Uint8>(g),
                    static_cast<Uint8>(b),
                    static_cast<Uint8>(a)
                };
            }
        }
    }

    inFile.close();
    return true;
}

bool Canvas::exportToPNG(const std::string& filepath) {
    // Generate combined 32-bit RGBA pixel stream
    std::vector<unsigned char> pixelBuffer;
    pixelBuffer.reserve(width * height * 4);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            SDL_Color blended = getBlendedPixel(x, y);
            pixelBuffer.push_back(blended.r);
            pixelBuffer.push_back(blended.g);
            pixelBuffer.push_back(blended.b);
            pixelBuffer.push_back(blended.a);
        }
    }

    // Call stb_image_write PNG exporter
    int result = stbi_write_png(
        filepath.c_str(), 
        width, 
        height, 
        4, 
        pixelBuffer.data(), 
        width * 4
    );

    return (result != 0);
}
