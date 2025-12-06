#ifndef EFFECTS_HPP
#define EFFECTS_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>



class ScreenShake {
public:
    float trauma = 0.f;
    float maxOffset = 10.f; 
    sf::Vector2f shakeDir = {1.f, 0.f};
    std::mt19937 rng;

    ScreenShake() : rng(std::random_device{}()) {}

    void addTrauma(float amount) {
        if (trauma <= 0.01f) {
            std::uniform_int_distribution<int> dist(0, 3);
            int dir = dist(rng);
            switch (dir) {
                case 0: shakeDir = {1.f, 0.f}; break; // Horizontal
                case 1: shakeDir = {0.f, 1.f}; break; // Vertical
                case 2: shakeDir = {0.707f, 0.707f}; break; // Diagonal
                case 3: shakeDir = {-0.707f, 0.707f}; break; // Diagonal
            }
        }
        trauma = std::min(trauma + amount, 1.f);
    }

    void update(float dt) {
        if (trauma > 0.f) {
            trauma -= dt * 1.5f; // Decay rate
            if (trauma < 0.f) trauma = 0.f;
        }
    }

    void apply(sf::View& view, float seed) {
        if (trauma > 0.f) {
            float shake = trauma * trauma; // Quadratic falloff
            float offset = maxOffset * shake * (2.f * noise(seed) - 1.f);
            view.move(shakeDir * offset);
        }
    }

private:
    float noise(float s) {
        return static_cast<float>(std::sin(s * 12.9898f + 78.233f) * 43758.5453f) - std::floor(static_cast<float>(std::sin(s * 12.9898f + 78.233f) * 43758.5453f));
    }
};

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
    sf::Color color;
};

class ParticleSystem {
public:
    std::vector<Particle> particles;
    std::mt19937 rng;

    ParticleSystem() : rng(std::random_device{}()) {}

    void emit(sf::Vector2f position, int count, sf::Color color, float speed = 100.f) {
        std::uniform_real_distribution<float> angleDist(0.f, 2.f * 3.14159f);
        std::uniform_real_distribution<float> speedDist(speed * 0.5f, speed * 1.5f);
        std::uniform_real_distribution<float> lifeDist(0.3f, 0.8f);

        for (int i = 0; i < count; ++i) {
            float angle = angleDist(rng);
            float s = speedDist(rng);
            sf::Vector2f vel = {std::cos(angle) * s, std::sin(angle) * s};
            particles.push_back({position, vel, lifeDist(rng), lifeDist(rng), color});
        }
    }

    void update(float dt) {
        for (auto& p : particles) {
            p.lifetime -= dt;
            p.position += p.velocity * dt;
        }

        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.lifetime <= 0.f; }),
            particles.end());
    }

    void draw(sf::RenderWindow& window) {
        sf::VertexArray va(sf::PrimitiveType::Triangles, particles.size() * 6); 
        
        float size = 4.f; 

        for (size_t i = 0; i < particles.size(); ++i) {
            const auto& p = particles[i];
            
            sf::Color c = p.color;
            float alpha = (p.lifetime / p.maxLifetime) * 255.f;
            c.a = static_cast<std::uint8_t>(alpha);

            size_t idx = i * 6;
            
            float half = size / 2.f;
            sf::Vector2f pos = p.position;

            va[idx + 0].position = {pos.x - half, pos.y - half};
            va[idx + 1].position = {pos.x + half, pos.y - half};
            va[idx + 2].position = {pos.x - half, pos.y + half};
            
            va[idx + 3].position = {pos.x + half, pos.y - half};
            va[idx + 4].position = {pos.x + half, pos.y + half};
            va[idx + 5].position = {pos.x - half, pos.y + half};

            for(int j=0; j<6; ++j) va[idx+j].color = c;
        }
        window.draw(va);
    }
};


class HitSplash {
public:
    sf::Sprite sprite;
    inline static sf::Texture t1, t2, t3;
    inline static bool texturesLoaded = false;
    float timer = 0.f;
    int frame = 0;
    float frameDuration = 0.1f; // Speed of animation

    HitSplash(sf::Vector2f position, float angle) : sprite(t1) {
        if (!texturesLoaded) {
            if (!t1.loadFromFile("resources/HitSplash/HS1.png")) std::cerr << "Failed to load HS1.png\n";
            if (!t2.loadFromFile("resources/HitSplash/HS2.png")) std::cerr << "Failed to load HS2.png\n";
            if (!t3.loadFromFile("resources/HitSplash/HS3.png")) std::cerr << "Failed to load HS3.png\n";
            texturesLoaded = true;
        }
        sprite.setTexture(t1, true);
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        sprite.setPosition(position);
        sprite.setRotation(sf::degrees(angle));
        sprite.setScale({0.5f, 0.5f}); // Adjust scale if needed
    }

    void setPosition(sf::Vector2f pos) {
        sprite.setPosition(pos);
    }

    void setRotation(float angle) {
        sprite.setRotation(sf::degrees(angle));
    }

    void update(float dt) {
        timer += dt;
        if (timer >= frameDuration) {
            timer = 0.f;
            frame++;
            if (frame > 2) frame = 0; // Loop animation
            
            if (frame == 0) sprite.setTexture(t1, true);
            else if (frame == 1) sprite.setTexture(t2, true);
            else if (frame == 2) sprite.setTexture(t3, true);
        }
    }
};

class ShockwaveRipple {
public:
    sf::CircleShape shape;
    float lifetime = 0.f;
    float maxLifetime = 1.5f; // Slower duration
    float maxRadius = 1500.f; // Larger radius to ensure coverage
    float speed;
    float currentThickness = 30.f;
    float currentAlpha = 255.f;

    ShockwaveRipple(sf::Vector2f center) {
        shape.setPosition(center);
        shape.setRadius(10.f);
        shape.setOrigin({10.f, 10.f});
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color(0, 255, 255, 255)); // Cyan
        shape.setOutlineThickness(30.f); 
        
        // Calculate speed to reach maxRadius in maxLifetime
        speed = maxRadius / maxLifetime;
    }

    bool update(float dt) {
        lifetime += dt;
        if (lifetime >= maxLifetime) return false;

        float progress = lifetime / maxLifetime;
        // Non-linear expansion for "powerful" feel
        float currentRadius = speed * lifetime; 
        
        shape.setRadius(currentRadius);
        shape.setOrigin({currentRadius, currentRadius});

        // Fade out
        currentAlpha = 255.f * (1.f - std::pow(progress, 2.f));
        currentThickness = 30.f * (1.f - progress);

        return true;
    }

    void draw(sf::RenderWindow& window) {
        // Neon Glow Effect using Additive Blending
        
        // Pass 1: Wide, faint glow
        shape.setOutlineThickness(currentThickness * 2.5f);
        sf::Color glowColor = sf::Color(0, 255, 255);
        glowColor.a = static_cast<std::uint8_t>(currentAlpha * 0.3f);
        shape.setOutlineColor(glowColor);
        window.draw(shape, sf::BlendAdd);

        // Pass 2: Core, bright ring
        shape.setOutlineThickness(currentThickness);
        sf::Color coreColor = sf::Color(200, 255, 255); // Slightly whiter for core
        coreColor.a = static_cast<std::uint8_t>(currentAlpha);
        shape.setOutlineColor(coreColor);
        window.draw(shape, sf::BlendAdd);
        
        // Pass 3: Inner white hot core (optional, thin)
        shape.setOutlineThickness(currentThickness * 0.3f);
        sf::Color hotColor = sf::Color::White;
        hotColor.a = static_cast<std::uint8_t>(currentAlpha);
        shape.setOutlineColor(hotColor);
        window.draw(shape, sf::BlendAdd);
    }
};

#include <deque>

// ... (existing includes)

class FloatingText {
public:
    sf::Text text;
    float lifetime;
    float maxLifetime;
    sf::Vector2f velocity;

    FloatingText(const sf::Font& font, const std::string& str, sf::Vector2f position, sf::Color color, float duration = 1.f)
        : text(font), lifetime(duration), maxLifetime(duration) {
        text.setString(str);
        text.setCharacterSize(20);
        text.setFillColor(color);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(1.f);
        
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        text.setPosition(position);
        
        // Random slight horizontal drift, upward movement
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> drift(-30.f, 30.f);
        velocity = {drift(rng), -50.f}; // Move up
    }

    bool update(float dt) {
        lifetime -= dt;
        if (lifetime <= 0.f) return false;

        text.move(velocity * dt);
        
        float alpha = (lifetime / maxLifetime) * 255.f;
        sf::Color c = text.getFillColor();
        c.a = static_cast<std::uint8_t>(alpha);
        text.setFillColor(c);
        
        sf::Color ox = text.getOutlineColor();
        ox.a = static_cast<std::uint8_t>(alpha);
        text.setOutlineColor(ox);

        return true;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(text);
    }
};

class Trail {
public:
    std::deque<sf::Vector2f> points;
    float timer = 0.f;
    float interval = 0.02f; // Frequent sampling for smoothness
    int maxPoints = 20;

    void update(sf::Vector2f pos, float dt) {
        timer += dt;
        if (timer >= interval) {
            timer = 0.f;
            points.push_front(pos);
            if (points.size() > maxPoints) {
                points.pop_back();
            }
        }
    }

    void clear() {
        points.clear();
        timer = 0.f;
    }

    void draw(sf::RenderWindow& window, sf::Color color) {
        if (points.empty()) return;

        sf::VertexArray va(sf::PrimitiveType::LineStrip, points.size());
        for (std::size_t i = 0; i < points.size(); ++i) {
            va[i].position = points[i];
            
            // Alpha fade
            float ratio = 1.f - static_cast<float>(i) / points.size();
            sf::Color c = color;
            c.a = static_cast<std::uint8_t>(ratio * 150.f); 
            va[i].color = c;
        }
        window.draw(va);
    }
};



#endif
