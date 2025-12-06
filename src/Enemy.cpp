#include "Enemy.hpp"
#include "window.hpp"
#include <random>
#include <cmath>
#include <algorithm>

static std::mt19937 rng(std::random_device{}());
static std::uniform_int_distribution<int> randOffset(-100, 100);

sf::Texture Enemy::textureInitial;
sf::Texture Enemy::textureExplosion;
bool Enemy::texturesLoaded = false;

void Enemy::loadTextures() {
    if (!texturesLoaded) {
        if (!textureInitial.loadFromFile("resources/enemy_Initial.png")) {
            // Handle error or use fallback
        }
        if (!textureExplosion.loadFromFile("resources/Enemy_BeforeExplosion.png")) {
            // Handle error
        }
        texturesLoaded = true;
    }
}

Enemy::Enemy(const Window &Game) : body(textureInitial) {
    loadTextures();
    // Scale sprite to match desired dimensions (assuming texture is roughly square or we want to stretch)
    sf::Vector2u texSize = textureInitial.getSize();
    body.setScale({width / texSize.x, height / texSize.y});
    
    body.setOrigin({texSize.x / 2.f, texSize.y / 2.f});
    body.setColor(color);
    HP = maxHp;
    respawn(Game.worldView, 50.f);
}

void Enemy::respawn(const sf::View &view, float margin) {
    std::uniform_real_distribution<float> choice(0.f, 1.f);
    sf::Vector2f viewSize = view.getSize();
    std::uniform_real_distribution<float> horizontal(0.f, viewSize.x + 2.f * margin);
    std::uniform_real_distribution<float> vertical(0.f, viewSize.y + 2.f * margin);

    sf::Vector2f viewCenter = view.getCenter();
    float left = viewCenter.x - viewSize.x / 2.f - margin;
    float top = viewCenter.y - viewSize.y / 2.f - margin;
    float right = viewCenter.x + viewSize.x / 2.f + margin;
    float bottom = viewCenter.y + viewSize.y / 2.f + margin;

    sf::Vector2f spawnPos = viewCenter;

    if (choice(rng) < 0.5f)
    {
        spawnPos.x = (choice(rng) < 0.5f) ? left : right;
        spawnPos.y = top + vertical(rng);
    }
    else
    {
        spawnPos.y = (choice(rng) < 0.5f) ? top : bottom;
        spawnPos.x = left + horizontal(rng);
    }

    body.setPosition(spawnPos);
    trail.clear();
    HP = maxHp;
}

void Enemy::applyDifficulty(float difficulty){
    float clamped = std::clamp(difficulty, 0.5f, 5.f);
    speed = baseSpeed * clamped;
}
