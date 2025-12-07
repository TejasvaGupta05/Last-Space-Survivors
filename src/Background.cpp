#include "Background.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

Background::Background(const std::string& resourcePath) {
    srand(time(nullptr));
    seed = rand();
    loadTextures(resourcePath);

    // Initialize Layers
    // Layer 0: Deep Background (Stars) - Slowest
    layers.push_back({0.05f, {}});
    
    // Layer 1: Mid Background (Stars) - Medium speed
    layers.push_back({0.1f, {}});

    // Layer 2: Foreground (Planets) - Fastest (but still slower than camera)
    layers.push_back({0.2f, {}});

    // Layer 3: Dust/Speed Particles - Moves with camera (Parallax 1.0 or near)
    layers.push_back({0.8f, {}});
    
    // Create 1x1 white texture
    sf::Image whiteImg;
    whiteImg.resize({1, 1}, sf::Color::White);
    if (!whiteTexture.loadFromImage(whiteImg)) {
        // Fallback?
    }
}

void Background::loadTextures(const std::string& resourcePath) {
    // Load Stars
    for (int i = 1; i <= 5; ++i) {
        sf::Texture tex;
        std::string path = resourcePath + "/Stars/Star_" + std::to_string(i) + ".png";
        if (tex.loadFromFile(path)) {
            starTextures.push_back(std::move(tex));
        } else {
            std::cerr << "Failed to load: " << path << std::endl;
        }
    }

    // Load Planets
    for (int i = 1; i <= 9; i++) {
        sf::Texture tex;
        std::string path = resourcePath + "/Planets/Planet_" + std::to_string(i) + ".png";
        if (tex.loadFromFile(path)) {
            planetTextures.push_back(std::move(tex));
        } else {
            std::cerr << "Failed to load: " << path << std::endl;
        }
    }
}

void Background::update(const sf::Vector2f& cameraPos, const sf::Vector2f& viewSize) {
    for (int i = 0; i < layers.size(); ++i) {
        sf::Vector2f virtualPos = cameraPos * layers[i].parallaxFactor;
        
        int cx = static_cast<int>(std::floor(virtualPos.x / chunkSize));
        int cy = static_cast<int>(std::floor(virtualPos.y / chunkSize));

        // Calculate radius based on view size (scaled by parallax factor? No, view size is constant)
        // But since we move slower, we might need to cover same area. 
        // Actually, the chunk grid is static, we just view a different part of it.
        // So standard radius logic applies to the virtual position.
        
        int radiusX = static_cast<int>(std::ceil(viewSize.x / chunkSize)) / 2 + 2;
        int radiusY = static_cast<int>(std::ceil(viewSize.y / chunkSize)) / 2 + 2;
        int radius = std::max(radiusX, radiusY);

        // Generate new chunks
        for (int x = cx - radius; x <= cx + radius; x++) {
            for (int y = cy - radius; y <= cy + radius; y++) {
                if (!layers[i].chunks.contains({x, y})) {
                    generateChunk(x, y, i);
                }
            }
        }

        // Remove old chunks
        removeFarChunks(cx, cy, radius + 1, i);
    }
}

void Background::generateChunk(int cx, int cy, int layerIndex) {
    auto chunk = std::make_unique<Chunk>();
    chunk->gridPos = {cx, cy};

    // Deterministic seed based on chunk coordinates, layer, and game seed
    std::seed_seq seq{cx, cy, layerIndex, static_cast<int>(seed)};
    std::vector<unsigned> seeds(1);
    seq.generate(seeds.begin(), seeds.end());
    std::mt19937 rng(seeds[0]);

    std::uniform_int_distribution<int> posDist(0, chunkSize);
    
    // --- Stars (Layers 0 and 1) ---
    if (layerIndex == 0 || layerIndex == 1) {
        std::uniform_int_distribution<int> starCountDist(12, 15); // Split density across layers
        int starCount = starCountDist(rng);

        if (!starTextures.empty()) {
            for (int i = 0; i < starCount; i++) {
                const sf::Texture& tex = starTextures[rng() % starTextures.size()];
                sf::Sprite star(tex);
                
                float lx = static_cast<float>(posDist(rng));
                float ly = static_cast<float>(posDist(rng));
                float wx = cx * chunkSize + lx;
                float wy = cy * chunkSize + ly;

                star.setPosition({wx, wy});

                // Scale variation based on layer (deeper = smaller)
                float minScale = (layerIndex == 0) ? 0.1f : 0.2f;
                float maxScale = (layerIndex == 0) ? 0.3f : 0.5f;
                std::uniform_real_distribution<float> scaleDist(minScale, maxScale);
                
                float scale = scaleDist(rng);
                star.setScale({scale, scale});

                // Color variation
                std::uniform_int_distribution<int> alphaDist(150, 255);
                std::uniform_int_distribution<int> colorTint(200, 255);
                star.setColor(sf::Color(colorTint(rng), colorTint(rng), colorTint(rng), alphaDist(rng)));

                chunk->sprites.push_back(star);
            }
        }
    }

    // --- Planets (Layer 2) ---
    if (layerIndex == 2) {
        if (!planetTextures.empty()) {
            std::uniform_real_distribution<float> planetChance(0.0f, 1.0f);
            if (planetChance(rng) < 0.5f) { // 50% chance
                const sf::Texture& tex = planetTextures[rng() % planetTextures.size()];
                sf::Sprite planet(tex);

                planet.setOrigin({tex.getSize().x * 0.5f, tex.getSize().y * 0.5f});

                float lx = static_cast<float>(posDist(rng));
                float ly = static_cast<float>(posDist(rng));
                float wx = cx * chunkSize + lx;
                float wy = cy * chunkSize + ly;

                planet.setPosition({wx, wy});

                std::uniform_real_distribution<float> scaleDist(1.f, 1.5f);
                float scale = scaleDist(rng);
                planet.setScale({scale, scale});
                
                std::uniform_real_distribution<float> rotDist(0.f, 360.f);
                planet.setRotation(sf::degrees(rotDist(rng)));

                planet.setColor(sf::Color(220, 220, 220, 255));

                chunk->sprites.push_back(planet);
            }
        }
    }

    // --- Dust (Layer 3) ---
    if (layerIndex == 3) {
        std::uniform_int_distribution<int> dustCountDist(8, 12);
        int dustCount = dustCountDist(rng);
        
        for (int i = 0; i < dustCount; i++) {
            sf::Sprite dust(whiteTexture);
            
            float lx = static_cast<float>(posDist(rng));
            float ly = static_cast<float>(posDist(rng));
            float wx = cx * chunkSize + lx;
            float wy = cy * chunkSize + ly;

            dust.setPosition({wx, wy});
            dust.setOrigin({0.5f, 0.5f});
            
            // Random opacity
            std::uniform_int_distribution<int> alphaDist(50, 150);
            dust.setColor(sf::Color(255, 255, 255, alphaDist(rng)));
            
            // Tiny base scale
            dust.setScale({2.f, 2.f}); // 2x2 pixel

            chunk->sprites.push_back(dust);
        }
    }

    layers[layerIndex].chunks[{cx, cy}] = std::move(chunk);
}

void Background::removeFarChunks(int cx, int cy, int radius, int layerIndex) {
    std::erase_if(layers[layerIndex].chunks, [&](const auto& item) {
        auto& [key, chunk] = item;
        int dx = std::abs(key.first - cx);
        int dy = std::abs(key.second - cy);
        return dx > radius || dy > radius;
    });
}

void Background::draw(sf::RenderWindow& window, sf::Vector2f playerVelocity) {
    sf::View originalView = window.getView();
    sf::Vector2f center = originalView.getCenter();

    // Calculate trail effect parameters
    sf::Vector2f dir = -playerVelocity; // Trail opposite to movement
    float speedSq = dir.x*dir.x + dir.y*dir.y;
    float speed = std::sqrt(speedSq);
    
    float stretchFactor = 1.f;
    sf::Angle angle = sf::degrees(0.f);
    
    // Stretch logic
    // Player speed is usually 0-12 pixels/frame
    if (speed > 0.5f) {
        stretchFactor = speed * 1.5f; 
        if (stretchFactor > 20.f) stretchFactor = 20.f; // Cap
        angle = sf::degrees(std::atan2(dir.y, dir.x) * 180.f / 3.14159f);
    }

    for (int i = 0; i < layers.size(); ++i) {
        sf::View layerView = originalView;
        layerView.setCenter(center * layers[i].parallaxFactor);
        window.setView(layerView);

        for (const auto& [key, chunk] : layers[i].chunks) {
            for (auto& sprite : chunk->sprites) { 
                // Note: iterating by non-const reference to modify sprite for drawing (if we wanted to modify state persistently)
                // But we shouldn't modify persistent state for temporary visual effect if we want them to return to dots.
                // However, sprites are stateful. We must reset or modify them.
                // Since this is immediate mode draw, we can modify -> draw -> reset, OR
                // Just force the transform we want every frame.
                
                if (i == 3) { // Dust Layer
                    // Apply trail effect
                     if (speed > 0.5f) {
                         sprite.setRotation(angle);
                         sprite.setScale({2.f + stretchFactor, 2.f}); // Stretch X, keep Y thin
                     } else {
                         sprite.setRotation(sf::degrees(0.f));
                         sprite.setScale({2.f, 2.f}); // Back to dot
                     }
                }
                
                window.draw(sprite);
            }
        }
    }

    window.setView(originalView);
}
