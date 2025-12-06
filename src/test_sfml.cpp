// Disabled to prevent "multiple definition of main" conflict with src/main.cpp
#if 0
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    sf::Font font;
    sf::Text text(font); // SFML 3 requires font in constructor
    sf::Vector2f v(10.f, 10.f);
    text.setOrigin(v);
    return 0;
}
#endif
