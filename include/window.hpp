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
        
        sf::Font UiFont; 

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
