#include "HUD.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

HUD::HUD(const sf::Font& font, const sf::Vector2f& windowSize)
    : font(font), windowSize(windowSize), coinText(font)
{
    // HUD Layout: Bottom Left
    // Padding from bottom edges
    float bottomMargin = 30.f;
    float barHeight = 20.f;
    float gap = 10.f;

    // --- Health Bar ---
    // Positioned at the top of the stack
    // Y = Height - margin - (3 bars heights) - (2 gaps)
    // Actually, let's stack them upwards from bottom.
    // Dash (Bottom), Energy (Middle), Health (Top)
    
    // Dash Bar (Bottom)
    sf::Vector2f dashSize(150.f, 10.f);
    sf::Vector2f dashPos(20.f, windowSize.y - bottomMargin - dashSize.y);
    // Color: Purple
    initBar(dashBar, dashPos, dashSize, sf::Color(162,25,255));

    // Energy Bar (Middle)
    sf::Vector2f enSize(200.f, 15.f);
    sf::Vector2f enPos(20.f, dashPos.y - gap - enSize.y);
    initBar(energyBar, enPos, enSize, sf::Color::Yellow);

    // Health Bar (Top)
    sf::Vector2f hpSize(200.f, 20.f);
    sf::Vector2f hpPos(20.f, enPos.y - gap - hpSize.y);
    initBar(healthBar, hpPos, hpSize, sf::Color::Red);

    // --- Shockwave Charges ---
    // Positioned above the Health Bar, Left Aligned
    float circleRadius = 10.f;
    for (int i = 0; i < 3; ++i) {
        sf::CircleShape circle(circleRadius);
        sf::Vector2f origin(circleRadius, circleRadius);
        circle.setOrigin(origin);
        
        // Start X should be same as hpPos.x (20.f) + radius
        // Y should be hpPos.y - padding - radius
        // Let's verify hpPos.y is calculated above: 
        // sf::Vector2f hpPos(20.f, enPos.y - gap - hpSize.y);
        
        // Let's use hpPos.x + radius + i * (diameter + gap)
        float startX = hpPos.x + circleRadius;
        float startY = hpPos.y - 20.f; // 15px above health bar

        sf::Vector2f pos(startX + i * 25.f, startY);
        circle.setPosition(pos); 
        
        // Color: Gray by default, Active will be Blue
        circle.setFillColor(sf::Color(100, 100, 100, 150));
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(2.f);
        shockwaveCircles.push_back(circle);
    }

    // --- Coins ---
    for (int i = 1; i <= 8; ++i) {
        sf::Texture tex;
        if (tex.loadFromFile("resources/Coin/Coin" + std::to_string(i) + ".png")) {
            coinTextures.push_back(tex);
        }
    }

    if (!coinTextures.empty()) {
        coinSprite.emplace(coinTextures[0]);
        sf::Vector2f scale(1.2f, 1.2f);
        coinSprite->setScale(scale);
        
        sf::FloatRect bounds = coinSprite->getLocalBounds();
        sf::Vector2f origin(bounds.size.x / 2.f, bounds.size.y / 2.f);
        coinSprite->setOrigin(origin);
    }

    coinText.setCharacterSize(32);
    coinText.setFillColor(sf::Color::White);
    coinText.setOutlineColor(sf::Color::Black);
    coinText.setOutlineThickness(1.f);
    coinText.setString("0");
    
    // Position Coin Top Right (Keep this as is?)
    // User only said "GUI position to bottom left", likely referring to the bars.
    // Coins generally stay top right or top left. I'll keep top right unless requested.
    sf::Vector2f coinPos(windowSize.x - 40.f, 30.f);
    if(coinSprite) coinSprite->setPosition(coinPos);
}

void HUD::initBar(HUD::Bar& bar, const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& fillColor) {
    bar.background.setPosition(position);
    bar.background.setSize(size);
    bar.background.setFillColor(sf::Color::Black);
    bar.background.setOutlineColor(sf::Color::White);
    bar.background.setOutlineThickness(2.f);

    bar.foreground.setPosition(position);
    bar.foreground.setSize(size); 
    bar.foreground.setFillColor(fillColor);
}

void HUD::updateBar(HUD::Bar& bar, float percentage, const sf::Color& baseColor) {
    percentage = std::clamp(percentage, 0.f, 1.f);
    sf::Vector2f fullSize = bar.background.getSize();
    sf::Vector2f newSize(fullSize.x * percentage, fullSize.y);
    bar.foreground.setSize(newSize);
    bar.foreground.setFillColor(baseColor);
}

void HUD::update(const Player& player, int coins, float dt) {
    // Update Health
    float hpPercent = player.getHealthPercent();
    sf::Color hpColor = sf::Color::Red;
    if (hpPercent > 0.5f) hpColor = sf::Color::Green;
    else if (hpPercent > 0.25f) hpColor = sf::Color(255, 165, 0); 

    updateBar(healthBar, hpPercent, hpColor);

    // Update Energy
    float energyPercent = player.getLaserEnergyPercent();
    sf::Color energyColor = sf::Color::Yellow;
    if (player.isOverheated) energyColor = sf::Color(255, 50, 50); 
    updateBar(energyBar, energyPercent, energyColor);

    // Update Dash
    float nitroPercent = player.getNitroPercent();
    float dashReady = nitroPercent; 
    // Dash Bar Color: Purple
    updateBar(dashBar, dashReady, sf::Color(162,25,255));

    // Update Shockwave Charges
    for (int i = 0; i < 3; ++i) {
        if (i < player.shockwaveCharges) {
            // Active Color: Blue
            shockwaveCircles[i].setFillColor(sf::Color::Blue);
        } else {
            shockwaveCircles[i].setFillColor(sf::Color(50, 50, 50, 150));
        }
    }

    // Update Coin Logic
    if (!coinTextures.empty() && coinSprite) {
        coinAnimTimer += dt;
        if (coinAnimTimer >= 0.1f) {
            coinAnimTimer = 0.f;
            currentCoinFrame = (currentCoinFrame + 1) % coinTextures.size();
            coinSprite->setTexture(coinTextures[currentCoinFrame]);
        }
    }
    
    coinText.setString(std::to_string(coins));
    sf::FloatRect textBounds = coinText.getLocalBounds();
    sf::Vector2f textOrigin(textBounds.size.x, textBounds.size.y / 2.f);
    coinText.setOrigin(textOrigin); 
    
    if(coinSprite) {
        sf::Vector2f iconPos = coinSprite->getPosition();
        sf::Vector2f textPos(iconPos.x - 35.f, iconPos.y - 5.f);
        coinText.setPosition(textPos); 
    }
}

void HUD::draw(sf::RenderWindow& window) {
    window.draw(healthBar.background);
    window.draw(healthBar.foreground);

    window.draw(energyBar.background);
    window.draw(energyBar.foreground);
    
    window.draw(dashBar.background);
    window.draw(dashBar.foreground);

    for (const auto& circle : shockwaveCircles) {
        window.draw(circle);
    }
    
    if(coinSprite) window.draw(*coinSprite);
    window.draw(coinText);
}
