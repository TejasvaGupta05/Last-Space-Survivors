#include "TitleScreen.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

TitleScreen::TitleScreen() : titleSprite(titleTexture), clickSound(clickBuffer), selectedOption(0), time(0.f), creditsText(font), backText(font), howToPlayText(font) {
    optionLabels = {"PLAY", "HOW TO PLAY", "CREDITS", "EXIT"};
}

bool TitleScreen::init(float width, float height) {
    // Try to load custom font
    if (!font.openFromFile("resources/Font/Jumps Winter.ttf")) {
        // Fallback or error handling
        std::cerr << "Failed to load font resources/Font/Jumps Winter.ttf" << std::endl;
        return false;
    }

    // Load sound
    if (!clickBuffer.loadFromFile("resources/UI_Button.wav")) {
        std::cerr << "Failed to load sound resources/UI_Button.wav" << std::endl;
        // We continue even if sound fails, just don't play it
    } else {
        clickSound.setBuffer(clickBuffer);
    }

    // Setup Title
    if (!titleTexture.loadFromFile("resources/Title.png")) {
        std::cerr << "Failed to load resources/Title.png" << std::endl;
        return false;
    }
    titleSprite.setTexture(titleTexture, true);
    
    sf::FloatRect titleBounds = titleSprite.getLocalBounds();
    titleSprite.setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
    titleSprite.setPosition({width / 2.f, height / 4.f});

    // Setup Options
    for (size_t i = 0; i < optionLabels.size(); ++i) {
        // Main Option Text
        sf::Text text(font);
        text.setString(optionLabels[i]);
        text.setCharacterSize(50);
        
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
        text.setPosition({width / 2.f, height / 2.f + i * 80.f});
        
        options.push_back(text);
    }
    
    // Setup Credits Text
    creditsText.setFont(font);
    creditsText.setString("A Game By : Tejasva Gupta");
    creditsText.setCharacterSize(40);
    creditsText.setFillColor(sf::Color::White);
    sf::FloatRect cBounds = creditsText.getLocalBounds();
    creditsText.setOrigin({cBounds.size.x / 2.f, cBounds.size.y / 2.f});
    creditsText.setPosition({width / 2.f, height / 2.f});

    // Setup How To Play Text
    howToPlayText.setFont(font);
    howToPlayText.setString(
        "CONTROLS\n\n"
        "WASD / ARROWS  -  MOVE\n"
        "LEFT CLICK     -  SHOOT LASER\n"
        "RIGHT CLICK    -  SHOCKWAVE\n"
        "SPACE          -  NITRO BOOST\n"
        "R              -  REPAIR (500)\n"
    );
    howToPlayText.setCharacterSize(30);
    howToPlayText.setFillColor(sf::Color::White);
    howToPlayText.setLineSpacing(1.5f);
    sf::FloatRect htpBounds = howToPlayText.getLocalBounds();
    howToPlayText.setOrigin({htpBounds.size.x / 2.f, htpBounds.size.y / 2.f});
    howToPlayText.setPosition({width / 2.f, height / 2.f});
    
    backText.setFont(font);
    backText.setString("Press any key to return");
    backText.setCharacterSize(20);
    backText.setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect bBounds = backText.getLocalBounds();
    backText.setOrigin({bBounds.size.x / 2.f, bBounds.size.y / 2.f});
    backText.setPosition({width / 2.f, height - 50.f});

    return true;
}

int TitleScreen::handleInput(sf::Keyboard::Key key) {
    if (showingCredits || showingHowToPlay) {
        showingCredits = false;
        showingHowToPlay = false;
        clickSound.play();
        return -1;
    }


    if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::W) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = static_cast<int>(options.size()) - 1;
    } else if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::S) {
        selectedOption++;
        if (selectedOption >= static_cast<int>(options.size())) selectedOption = 0;
    } else if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space) {
        clickSound.play();
        if (selectedOption == 1) { // How to Play
            showingHowToPlay = true;
            return -1;
        }
        if (selectedOption == 2) { // Credits
            showingCredits = true;
            return -1;
        }
        return selectedOption;
    }
    return -1;
}

void TitleScreen::handleMouseMove(sf::Vector2f mousePos) {
    if (showingCredits || showingHowToPlay) return;
    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].getGlobalBounds().contains(mousePos)) {
            selectedOption = static_cast<int>(i);
            return;
        }
    }
}

int TitleScreen::handleClick(sf::Vector2f mousePos) {
    if (showingCredits || showingHowToPlay) {
        showingCredits = false;
        showingHowToPlay = false;
        clickSound.play();
        return -1;
    }
    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].getGlobalBounds().contains(mousePos)) {
            clickSound.play();
            if (i == 1) { // How to Play
                showingHowToPlay = true;
                return -1;
            }
            if (i == 2) { // Credits
                showingCredits = true;
                return -1;
            }
            return static_cast<int>(i);
        }
    }
    return -1;
}

void TitleScreen::update(float dt) {
    time += dt;
    
    // Pulse effect removed
    // float scale = 1.0f + 0.05f * std::sin(time * 2.f);
    // titleSprite.setScale({scale, scale});

    // Update options styling
    for (size_t i = 0; i < options.size(); ++i) {
        if (static_cast<int>(i) == selectedOption) {
            options[i].setFillColor(sf::Color::Yellow);
            options[i].setStyle(sf::Text::Bold);
        } else {
            options[i].setFillColor(sf::Color::White);
            options[i].setStyle(sf::Text::Regular);
        }
    }
}

void TitleScreen::draw(sf::RenderWindow& window) {
    if (showingCredits) {
        window.draw(creditsText);
        window.draw(backText);
    } else if (showingHowToPlay) {
        window.draw(howToPlayText);
        window.draw(backText);
    } else {
        window.draw(titleSprite);
        for (const auto& option : options) {
            window.draw(option);
        }
    }
}
