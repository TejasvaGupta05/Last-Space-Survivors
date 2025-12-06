#ifndef ENEMY_HPP
#define ENEMY_HPP
#include <SFML/Graphics.hpp>
#include <cmath>
#include "window.hpp"
#include "effects.hpp"

class Enemy {
public:
    float width = 72.f;
    float height = 72.f;
    float speed = 2.1f;
    float baseSpeed = 2.1f;
    float explosionDamage = 30.f; 
    sf::Color color = sf::Color::White;
    sf::Sprite body;
    Trail trail; // Enemy Trail
    static sf::Texture textureInitial;
    static sf::Texture textureExplosion;
    static bool texturesLoaded;
    int maxHp = 100;
    int HP = 100;
    
    // Default constructor must initialize body with a texture
    Enemy() : body(textureInitial) {}
    
    Enemy(const Window &Game);
    static void loadTextures();
    
    void update(float dt) {
        trail.update(body.getPosition(), dt);
    }

    void moveTowards(const sf::Vector2f& playerPos) {
        sf::Vector2f pos = body.getPosition();
        sf::Vector2f dir = playerPos - pos;
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.0001f) {
            dir.x /= len;
            dir.y /= len;
            body.move(dir * speed);
        }
    }

    void takeDamage(int dmg){
        HP -= dmg;
        if(HP < 0) HP = 0;
    }

    bool isDead() const { return HP <= 0; }

    void applyKnockback(const sf::Vector2f &dir, float force){
        body.move(dir * force);
    }
    void updateVisualState(sf::Vector2f playerPos) {
        sf::Vector2f pos = body.getPosition();
        //calculate the distance between enemy and player
        if(std::sqrt((playerPos.x - pos.x)*(playerPos.x - pos.x) + (playerPos.y - pos.y)*(playerPos.y - pos.y)) < 209.f) {
            body.setTexture(textureExplosion); // Change texture when close to player
        } else {
            body.setTexture(textureInitial); // Reset to original texture
        }
    }
    bool shouldExplode(const sf::Vector2f& playerPos) const {
        sf::Vector2f pos = body.getPosition();
        float distanceSquared = (playerPos.x - pos.x)*(playerPos.x - pos.x) + (playerPos.y - pos.y)*(playerPos.y - pos.y);
        return distanceSquared < (100.f * 100.f); // Explode if within 30 units
    }
    void applyShockwave(const sf::Vector2f& playerPos, float shockwaveForce,float time){

    }
    void respawn(const sf::View &view, float margin);
    void applyDifficulty(float difficulty);
};

#endif