#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Angle.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>

class Player{
    public:
        float width = 100.f;
        float height = 100.f;
        float accX = 0.f;
        float accY = 0.f;
        float velX = 0.f;
        float velY = 0.f;
        float baseMaxSpeed = 6.f;
        float nitroMaxSpeed = 12.f;
        float maxSpeed = 6.f;
        float baseFriction = 0.08f;
        float nitroFrictionFactor = 0.25f;
        float nitroAccelMultiplier = 2.f;
        float baseAcceleration = 28.f;
        int shootCooldown = 0;
        float nitroCapacity = 120.f;
        float nitroCharge = nitroCapacity;
        float nitroDrainRate = 45.f;
        float nitroRechargeRate = 22.f;
        bool nitroActive = false;
        
        sf::Color color = sf::Color::Green;
        
        // Textures must be declared before Sprite to ensure they are initialized first
        sf::Texture textureInitial;
        sf::Texture textureAccel;
        sf::Texture textureBoost;
        sf::Sprite body;

        int maxHP = 1000;
        int HP = 1000;
        int shockwaveCharges = 0;
        const int maxShockwaveCharges = 3;
        bool shockwaveActive = false;
        float shockwaveDuration = 1.0f;  // 1 second
        float shockwaveTimer = 0.f;
        float shockwaveForce = 300.f;  // Force to repel enemies
        
        // Laser Energy System
        float laserEnergy = 100.f;
        float maxLaserEnergy = 100.f;
        float laserCost = 0.3f;
        float laserRechargeRate = 60.f;
        float laserRechargeDelay = 0.5f;
        float laserRechargeTimer = 0.f;
        bool isOverheated = false;
        
        Player(float startX, float startY) : body(textureInitial) {
            if (!textureInitial.loadFromFile("resources/Player/Initial.png")) {
                std::cerr << "Failed to load Initial.png" << std::endl;
            }
            if (!textureAccel.loadFromFile("resources/Player/OnAcceleration.png")) {
                std::cerr << "Failed to load OnAcceleration.png" << std::endl;
            }
            if (!textureBoost.loadFromFile("resources/Player/OnBoost.png")) {
                std::cerr << "Failed to load OnBoost.png" << std::endl;
            }

            body.setTexture(textureInitial, true);
            sf::FloatRect bounds = body.getLocalBounds();
            body.setOrigin({bounds.size.x / 2.f, (bounds.size.y / 2.f)-12.f});
            body.setPosition({startX,startY});
        }

        void updateTexture(bool isMoving) {
            if (nitroActive) {
                body.setTexture(textureBoost, true);
            } else if (isMoving) {
                body.setTexture(textureAccel, true);
            } else {
                body.setTexture(textureInitial, true);
            }
            // Re-center origin as texture size might change
            sf::FloatRect bounds = body.getLocalBounds();
            body.setOrigin({bounds.size.x / 2.f, (bounds.size.y / 2.f)-12.f});
        }

        void deaccelerate(float &vel,float &acc,bool forX=true,float friction=0.1f){
            vel += acc;
            if(forX) body.move({vel,0.f});
            else body.move({0.f,vel});
            acc = 0.f;
            if(vel != 0.f){
                if(vel > 0.f){
                    vel -= friction;
                    if(vel < 0.f) vel = 0.f;
                }else if(vel< 0.f){
                    vel += friction;
                    if(vel > 0.f) vel = 0.f;
                }
            }
        }
        void diagonalhandle(){
            float mag = std::sqrt(velX*velX + velY*velY);
            if(mag>maxSpeed){
                velX=(velX/mag)*maxSpeed;
                velY=(velY/mag)*maxSpeed;
            }
        }
        void RotateTowards(const sf::Vector2f& target){
            sf::Vector2f pos = body.getPosition();
            float dx = target.x - pos.x;
            float dy = target.y - pos.y;
            float angle = std::atan2f(dy, dx) * (180.f / 3.14159265f) + 90.f;
            body.setRotation(sf::degrees(angle));
        }
        bool updateNitro(bool engage, float dt){
            if(engage && nitroCharge > 0.f){
                nitroActive = true;
                nitroCharge -= nitroDrainRate * dt;
                if(nitroCharge <= 0.f){
                    nitroCharge = 0.f;
                    nitroActive = false;
                }
            }else{
                nitroActive = false;
                nitroCharge += nitroRechargeRate * dt;
                if(nitroCharge > nitroCapacity) nitroCharge = nitroCapacity;
            }
            return nitroActive;
        }
        float getNitroPercent() const{
            if(nitroCapacity <= 0.f) return 0.f;
            return nitroCharge / nitroCapacity;
        }
        float getHealthPercent() const{
            if(maxHP <= 0) return 0.f;
            return static_cast<float>(HP) / static_cast<float>(maxHP);
        }
        void takeDamage(int dmg){
            HP -= dmg;
            if(HP < 0) HP = 0;
        }
        bool isDead() const { return HP <= 0; }
        
        bool activateShockwave() {
            if(shockwaveCharges > 0 && !shockwaveActive) {
                shockwaveCharges--;
                shockwaveActive = true;
                shockwaveTimer = shockwaveDuration;
                return true;
            }
            return false;
        }
        
        void updateShockwave(float dt) {
            if(shockwaveActive) {
                shockwaveTimer -= dt;
                if(shockwaveTimer <= 0.f) {
                    shockwaveActive = false;
                    shockwaveTimer = 0.f;
                }
            }
        }
        
        void addShockwaveCharge() {
            if(shockwaveCharges < maxShockwaveCharges) {
                shockwaveCharges++;
            }
        }

        void updateLaserEnergy(float dt) {
            if (laserRechargeTimer > 0.f) {
                laserRechargeTimer -= dt;
            } else {
                if (laserEnergy < maxLaserEnergy) {
                    laserEnergy += laserRechargeRate * dt;
                    if (laserEnergy > maxLaserEnergy) {
                        laserEnergy = maxLaserEnergy;
                    }
                    
                    if (isOverheated && laserEnergy >= maxLaserEnergy * 0.5f) {
                        isOverheated = false; // Reset overheat at 50%
                    }
                }
            }
        }

        bool canShoot() const {
            return !isOverheated && laserEnergy > 0.f;
        }

        void consumeLaserEnergy() {
            laserEnergy -= laserCost;
            if (laserEnergy <= 0.f) {
                laserEnergy = 0.f;
                isOverheated = true; // Trigger overheat
            }
            laserRechargeTimer = laserRechargeDelay;
        }

        float getLaserEnergyPercent() const {
            if (maxLaserEnergy <= 0.f) return 0.f;
            return laserEnergy / maxLaserEnergy;
        }
        
};

class Laser{
    public:
        sf::Sprite body;
        inline static sf::Texture texture;
        inline static bool textureLoaded = false;
        inline static float initialLifetime = 0.15f; // Laser visible for 0.2 seconds
        float lifetime = initialLifetime;

        Laser(sf::Vector2f startPos, float angleDeg, float length) : body(texture) {
            if(!textureLoaded){
                if(!texture.loadFromFile("resources/Laser.png")){
                    std::cerr << "Failed to load Laser.png" << std::endl;
                }
                // texture.setRepeated(true); // Disable repeat to avoid artifacts
                textureLoaded = true;
            }
            // body.setTexture(texture); // Already set in initializer list
            
            // Crop the texture to the length required, but clamp to texture width (1400)
            int rectWidth = std::min(static_cast<int>(length), 1400);
            body.setTextureRect(sf::IntRect({0, 0}, {rectWidth, 150}));
            
            // Set origin to middle-left (start of beam)
            // Adjusted to 40.f to center the beam visually
            body.setOrigin({0.f, 17.f}); 
            
            body.setPosition(startPos);
            body.setRotation(sf::degrees(angleDeg));
            
            // Scale down height to make it a beam, keep width (length) scale 1
            // Increased thickness to 1.5f
            body.setScale({1.f, 1.f}); 
        }
        
        bool update(float dt){
            lifetime -= dt;
            // Fade out effect
            int alpha = static_cast<int>((lifetime / 0.1f) * 255);
            if(alpha < 0) alpha = 0;
            body.setColor(sf::Color(255, 255, 255, alpha));
            return lifetime > 0.f;
        }
};

class ShockwaveOrb{
    public:
        float speed = 8.f;
        float collectRadius = 30.f;  // Distance at which player can collect
        sf::CircleShape body;
        ShockwaveOrb(sf::Vector2f startPos){
            body.setRadius(8.f);
            body.setOrigin({8.f,8.f});
            body.setPosition(startPos);
            body.setFillColor(sf::Color::Cyan);
        }
        void update(const sf::Vector2f& playerPos){
            // Move towards player
            sf::Vector2f pos = body.getPosition();
            sf::Vector2f dir = playerPos - pos;
            float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
            if(len > 0.0001f) {
                dir.x /= len;
                dir.y /= len;
                body.move(dir * speed);
            }
        }
        bool isCollected(const sf::Vector2f& playerPos) const {
            sf::Vector2f pos = body.getPosition();
            sf::Vector2f diff = playerPos - pos;
            float distanceSquared = diff.x * diff.x + diff.y * diff.y;
            return distanceSquared <= (collectRadius * collectRadius);
        }
};

#endif
