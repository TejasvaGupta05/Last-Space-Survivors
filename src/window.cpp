#include "window.hpp" 

Window::Window(const std::string& title)
    : display(sf::VideoMode::getDesktopMode()),
      width(static_cast<float>(display.size.x)),
      height(static_cast<float>(display.size.y)),
      center(width / 2.f, height / 2.f),
      window(display, title, sf::State::Fullscreen),
      worldView(window.getDefaultView()),
      uiView(worldView)
{
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
    window.setView(worldView);

    if (!UiFont.openFromFile("resources/Font/Jumps Winter.ttf")) {
        // Handle error if needed, but for now we just log
    }
}
