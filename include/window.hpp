#ifndef WINDOW_HPP
#define WINDOW_HPP
#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <vector>
#include <optional>

class Window{
    public:
        sf::VideoMode display;
        const float width;
        const float height;
        sf::Vector2f center;
        sf::RenderWindow window;
        
        sf::Font UiFont; // Keep Font as it might be used by HUD or others (actually HUD loads its own ref, but Main might need it or pass it)
        // Actually HUD takes font by reference in constructor, Main loads it? 
        // In current main.cpp, Main accesses Game.UiFont? Let's see main.cpp again.
        // main.cpp doesn't fully load font, Window constructor loads it.
        // We should keep UiFont here to share it.

        sf::View worldView;
        sf::View uiView;
        Window(const std::string& title);

        sf::FloatRect getViewBounds(float margin = 0.f) const{
            sf::Vector2f viewCenter = worldView.getCenter();
            sf::Vector2f viewSize = worldView.getSize();
            sf::FloatRect bounds;
            bounds.position = {viewCenter.x - viewSize.x / 2.f - margin,
                               viewCenter.y - viewSize.y / 2.f - margin};
            bounds.size = {viewSize.x + 2.f * margin,
                           viewSize.y + 2.f * margin};
            return bounds;
        }
};

#endif
