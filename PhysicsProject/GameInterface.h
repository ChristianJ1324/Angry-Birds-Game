/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : GameInterface.h
Description : Draws the menu and game screens
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <array>
#include <cstddef>
#include <string>

enum class MenuAction
{
    None,
    Seesaw,
    Pulley,
    Exit
};

class GameInterface
{
public:
    // Menu input
    void LoadFont();
    MenuAction HandleMenuEvent(const sf::RenderWindow& window, const sf::Event& event);
    void CancelMenuPress()
    {
        pressedButton = -1;
    }

    // Screen drawing
    void DrawMenu(sf::RenderWindow& window, const std::array<sf::Texture, 3>& birdTextures);
    void DrawHud(sf::RenderWindow& window, const std::string& levelName,
        size_t enemies, size_t shot, size_t shotLimit, const char* birdDescription);
    void DrawResult(sf::RenderWindow& window, bool won);

private:
    struct MenuButton
    {
        std::string label;
        std::string detail;
        MenuAction action;
        float height;
        float centerOffsetY;
    };

    // Button layout and drawing
    sf::FloatRect GetButtonBounds(const sf::RenderWindow& window, size_t index) const;
    void DrawButton(sf::RenderWindow& window, size_t index);
    void DrawBadge(sf::RenderWindow& window, sf::Vector2f position,
        const std::string& text);
    void DrawPanel(sf::RenderWindow& window, sf::FloatRect rectBounds,
        float radius, sf::Color colour);
    void DrawText(sf::RenderWindow& window, const std::string& text,
        sf::Vector2f position, unsigned int size, sf::Color colour,
        bool centered = false, float maximumWidth = 0.0f);

    inline static const sf::Color panel{ 26, 46, 57, 245 };
    inline static const sf::Color mutedText{ 194, 214, 222 };
    inline static const sf::Color accent{ 255, 218, 128 };
    inline static const size_t cornerSteps = 12;
    inline static const float menuButtonWidth = 520.0f;
    inline static const float pi = 3.14159265f;
    inline static const std::array<MenuButton, 3> buttons{
        MenuButton{ "Tipping Towers", "Level 1 - Tip the seesaw to topple the tower", MenuAction::Seesaw, 76.0f, -40.0f },
        MenuButton{ "Pulley Yard", "Level 2 - Counterweight and tipping tray", MenuAction::Pulley, 76.0f, 52.0f },
        MenuButton{ "Exit", "", MenuAction::Exit, 64.0f, 148.0f } };
    sf::Font font;
    int pressedButton = -1;
};