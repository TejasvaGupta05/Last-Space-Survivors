#ifndef HUD_HPP
#define HUD_HPP

#include <SFML/Graphics.hpp>
#include "player.hpp"
#include <vector>
#include <string>
#include <optional>

class HUD {
public:
    HUD(const sf::Font& font, const sf::Vector2f& windowSize);

    void update(const Player& player, int coins, float dt);
    void draw(sf::RenderWindow& window);

private:
    const sf::Font& font;
    sf::Vector2f windowSize;

    // Helper to create a bar
    struct Bar {
        sf::RectangleShape background;
        sf::RectangleShape foreground;
        sf::RectangleShape border;
    };

    Bar healthBar;
    Bar energyBar;
    Bar dashBar;

    // Shockwave charges
    std::vector<sf::CircleShape> shockwaveCircles;

    // Coins
    // Coins
    sf::Text coinText;
    std::optional<sf::Sprite> coinSprite;
    std::vector<sf::Texture> coinTextures;
    int currentCoinFrame = 0;
    float coinAnimTimer = 0.f;

    // Initialization helpers
    void initBar(Bar& bar, const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& fillColor);
    void updateBar(Bar& bar, float percentage, const sf::Color& color);
};

#endif // HUD_HPP
