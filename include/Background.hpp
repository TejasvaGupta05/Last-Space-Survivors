#ifndef BACKGROUND_HPP
#define BACKGROUND_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <utility>
#include <memory>
#include <random>
#include <string>

class Background {
public:
    Background(const std::string& resourcePath);
    void update(const sf::Vector2f& cameraPos, const sf::Vector2f& viewSize);
    void draw(sf::RenderWindow& window, sf::Vector2f playerVelocity = {0.f, 0.f});

private:
    struct Chunk {
        sf::Vector2i gridPos;
        std::vector<sf::Sprite> sprites;
    };

    struct Layer {
        float parallaxFactor;
        std::map<std::pair<int, int>, std::unique_ptr<Chunk>> chunks;
    };

    int chunkSize = 1024;
    std::vector<Layer> layers;
    std::vector<sf::Texture> starTextures;
    std::vector<sf::Texture> planetTextures;
    sf::Texture whiteTexture;
    unsigned int seed;

    void loadTextures(const std::string& resourcePath);
    void generateChunk(int cx, int cy, int layerIndex);
    void removeFarChunks(int cx, int cy, int radius, int layerIndex);
};

#endif // BACKGROUND_HPP
