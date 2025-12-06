#ifndef TITLESCREEN_HPP
#define TITLESCREEN_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>

class TitleScreen {
public:
    TitleScreen();
    bool init(float width, float height);
    
    // Returns -1 for no selection, 0 for Play, 1 for Exit
    int handleInput(sf::Keyboard::Key key);
    void handleMouseMove(sf::Vector2f mousePos);
    int handleClick(sf::Vector2f mousePos);
    
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    sf::Font font;
    sf::Texture titleTexture;
    sf::Sprite titleSprite;
    std::vector<sf::Text> options;
    std::vector<std::string> optionLabels;
    int selectedOption;
    
    // Audio
    sf::SoundBuffer clickBuffer;
    sf::Sound clickSound;

    // Visual effects
    float time;

    // Credits
    bool showingCredits = false;
    sf::Text creditsText;
    sf::Text backText;
    
    // How to Play
    bool showingHowToPlay = false;
    sf::Text howToPlayText;
};

#endif
