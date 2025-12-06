// Disabled to prevent "multiple definition of main" conflict with src/main.cpp
// and to resolve SFML 3 compilation error (sf::Text requires a font).
#if 0
#include <SFML/Graphics.hpp>
int main() {
    sf::Text text;
    return 0;
}
#endif
