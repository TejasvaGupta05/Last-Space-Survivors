#ifndef COIN_HPP
#define COIN_HPP

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

class Coin {
public:
    sf::Sprite body;
    inline static sf::Texture texture;
    inline static bool textureLoaded = false;
    
    float magnetRadius = 250.f; // Distance to trigger attraction
    float speed = 600.f;       // Speed when flying to player
    float friction = 0.95f;    // To slow down if we wanted realistic physics, but here we just home in
    
    // Simple state
    bool magnetized = false;
    

    Coin(sf::Vector2f position) : body(texture) {
        if (!textureLoaded) {
            if (!texture.loadFromFile("resources/Coin/Coin3.png")) {
                std::cerr << "Failed to load resources/Coin/Coin3.png" << std::endl;
            }
            textureLoaded = true;
        }
        body.setTexture(texture);
        
        sf::FloatRect bounds = body.getLocalBounds();
        body.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        body.setPosition(position);
        
        // Scale to approx 12px diameter (original radius 6)
        // Check texture size? Usually 32x32. 12/32 = 0.375
        body.setScale({0.5f, 0.5f}); // Let's try 0.5 (16px) for better visibility
    }

    void update(const sf::Vector2f& playerPos, float dt) {
        sf::Vector2f pos = body.getPosition();
        sf::Vector2f diff = playerPos - pos;
        float distSquared = diff.x * diff.x + diff.y * diff.y;

        if (magnetized || distSquared < magnetRadius * magnetRadius) {
            magnetized = true;
            
            // Normalize direction
            float dist = std::sqrt(distSquared);
            if (dist > 0.001f) {
                sf::Vector2f dir = diff / dist;
                body.move(dir * speed * dt);
            }
        }
    }

    bool isCollected(const sf::Vector2f& playerPos) const {
        sf::Vector2f pos = body.getPosition();
        sf::Vector2f diff = playerPos - pos;
        float distSquared = diff.x * diff.x + diff.y * diff.y;
        // Collection radius (player radius approx 50 + coin radius 6 + fudge factor)
        return distSquared < (60.f * 60.f); 
    }

    bool isTooFar(const sf::Vector2f& playerPos, float threshold = 2500.f) const {
        sf::Vector2f pos = body.getPosition();
        sf::Vector2f diff = playerPos - pos;
        float distSquared = diff.x * diff.x + diff.y * diff.y;
        return distSquared > (threshold * threshold);
    }
    
    void draw(sf::RenderWindow& window) {
        window.draw(body);
    }
};

#endif
