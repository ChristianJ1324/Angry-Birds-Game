/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : GameInterface.cpp
Description : Draws the menu, HUD and result screen
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "GameInterface.h"
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>

void GameInterface::LoadFont()
{
    font.loadFromFile("Font/FingerPaint-Regular.ttf");
}

// Only activate a menu button when the press and release are on the same one
MenuAction GameInterface::HandleMenuEvent(const sf::RenderWindow& window,
    const sf::Event& event)
{
    if (event.type != sf::Event::MouseButtonPressed &&
        event.type != sf::Event::MouseButtonReleased)
    {
        return MenuAction::None;
    }
    if (event.mouseButton.button != sf::Mouse::Left)
    {
        return MenuAction::None;
    }

    const sf::Vector2f mouse = window.mapPixelToCoords(
        { event.mouseButton.x, event.mouseButton.y });
    int hitButton = -1;
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        if (GetButtonBounds(window, i).contains(mouse))
        {
            hitButton = static_cast<int>(i);
            break;
        }
    }
    if (event.type == sf::Event::MouseButtonPressed)
    {
        pressedButton = hitButton;
        return MenuAction::None;
    }

    const int buttonThatWasPressed = pressedButton;
    CancelMenuPress();
    return hitButton >= 0 && hitButton == buttonThatWasPressed ?
        buttons[static_cast<size_t>(hitButton)].action : MenuAction::None;
}

void GameInterface::DrawMenu(sf::RenderWindow& window,
    const std::array<sf::Texture, 3>& birdTextures)
{
    const sf::Vector2f center = window.getView().getCenter();
    DrawPanel(window, { center.x - 380.0f, center.y - 272.0f, 760.0f, 560.0f },
        24.0f, sf::Color(0, 0, 0, 35));
    DrawPanel(window, { center.x - 380.0f, center.y - 280.0f, 760.0f, 560.0f },
        24.0f, panel);
    DrawText(window, "ANGRY BIRDS", center + sf::Vector2f(0.0f, -218.0f),
        44, sf::Color::White, true, 680.0f);
    DrawText(window, "Choose a level", center + sf::Vector2f(0.0f, -166.0f),
        21, mutedText, true);
    for (size_t i = 0; i < birdTextures.size(); ++i)
    {
        const sf::Vector2u size = birdTextures[i].getSize();
        sf::Sprite bird(birdTextures[i]);
        bird.setOrigin(static_cast<float>(size.x) * 0.5f,
            static_cast<float>(size.y) * 0.5f);
        const float scale = 42.0f /
            static_cast<float>(std::max(size.x, size.y));
        bird.setScale(scale, scale);
        bird.setPosition(center +
            sf::Vector2f((static_cast<float>(i) - 1.0f) * 64.0f, -122.0f));
        window.draw(bird);
    }
    for (size_t i = 0; i < buttons.size(); ++i)
    {
        DrawButton(window, i);
    }
    DrawText(window, "Drag to launch - Space for ability",
        center + sf::Vector2f(0.0f, 236.0f), 16, mutedText, true, 680.0f);
}

void GameInterface::DrawHud(sf::RenderWindow& window, const std::string& levelName,
    size_t enemies, size_t shot, size_t shotLimit, const char* birdDescription)
{
    const sf::Vector2f size = window.getView().getSize();
    const sf::Vector2f origin = window.getView().getCenter() - size * 0.5f;
    sf::RectangleShape bar({ size.x, 88.0f });
    bar.setPosition(origin);
    bar.setFillColor(panel);
    window.draw(bar);
    sf::RectangleShape accentStripe({ size.x, 3.0f });
    accentStripe.setPosition(origin + sf::Vector2f(0.0f, 85.0f));
    accentStripe.setFillColor(accent);
    window.draw(accentStripe);
    DrawText(window, levelName, origin + sf::Vector2f(24.0f, 13.0f),
        23, sf::Color::White);
    DrawBadge(window, origin + sf::Vector2f(488.0f, 7.0f),
        "Enemies: " + std::to_string(enemies));
    DrawBadge(window, origin + sf::Vector2f(658.0f, 7.0f),
        "Bird " + std::to_string(shot) + " / " + std::to_string(shotLimit));
    DrawText(window, "R - Restart / Esc - Menu",
        origin + sf::Vector2f(size.x - 216.0f, 24.0f),
        15, mutedText, true, 360.0f);
    DrawText(window, birdDescription, origin + sf::Vector2f(24.0f, 57.0f),
        18, accent, false, size.x - 48.0f);
}

void GameInterface::DrawResult(sf::RenderWindow& window, bool won)
{
    const sf::Vector2f center = window.getView().getCenter();
    const sf::Vector2f size = window.getView().getSize();
    sf::RectangleShape overlay(size);
    overlay.setPosition(center - size * 0.5f);
    overlay.setFillColor(sf::Color(14, 25, 34, 165));
    window.draw(overlay);
    DrawPanel(window,
        { center.x - 360.0f, center.y - 110.0f, 720.0f, 220.0f },
        22.0f, panel);
    DrawText(window, won ? "LEVEL COMPLETE" : "OUT OF BIRDS",
        center + sf::Vector2f(0.0f, -35.0f), 36,
        won ? accent : sf::Color::White, true, 640.0f);
    DrawText(window, "Press R to Restart or Esc for Main menu",
        center + sf::Vector2f(0.0f, 42.0f), 20, mutedText, true, 640.0f);
}

sf::FloatRect GameInterface::GetButtonBounds(const sf::RenderWindow& window,
    size_t index) const
{
    const sf::Vector2f center = window.getView().getCenter();
    const MenuButton& button = buttons[index];
    return { center.x - menuButtonWidth * 0.5f,
        center.y + button.centerOffsetY - button.height * 0.5f,
        menuButtonWidth, button.height };
}

void GameInterface::DrawButton(sf::RenderWindow& window, size_t index)
{
    sf::FloatRect rectBounds = GetButtonBounds(window, index);
    const sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    const bool hovered = window.hasFocus() && rectBounds.contains(mouse);
    const bool pressed = hovered && pressedButton == static_cast<int>(index) &&
        sf::Mouse::isButtonPressed(sf::Mouse::Left);
    DrawPanel(window, { rectBounds.left, rectBounds.top + 5.0f,
        rectBounds.width, rectBounds.height }, 14.0f, sf::Color(0, 0, 0, 45));
    if (pressed)
    {
        rectBounds.top += 2.0f;
    }
    DrawPanel(window, rectBounds, 14.0f,
        hovered ? accent : sf::Color(86, 122, 137));
    sf::Color interiorColour(45, 75, 89);
    if (hovered)
    {
        interiorColour = sf::Color(62, 98, 113);
    }
    if (pressed)
    {
        interiorColour = sf::Color(41, 69, 83);
    }
    DrawPanel(window, { rectBounds.left + 2.0f, rectBounds.top + 2.0f,
        rectBounds.width - 4.0f, rectBounds.height - 4.0f }, 12.0f, interiorColour);
    const MenuButton& button = buttons[index];
    const sf::Vector2f center(rectBounds.left + rectBounds.width * 0.5f,
        rectBounds.top + rectBounds.height * 0.5f);
    DrawText(window, button.label,
        center + sf::Vector2f(0.0f, button.detail.empty() ? 0.0f : -12.0f),
        24, sf::Color::White, true, rectBounds.width - 40.0f);
    if (!button.detail.empty())
    {
        DrawText(window, button.detail,
            center + sf::Vector2f(0.0f, 20.0f), 14, mutedText, true,
            rectBounds.width - 40.0f);
    }
}

void GameInterface::DrawBadge(sf::RenderWindow& window, sf::Vector2f position,
    const std::string& text)
{
    const sf::Vector2f size(152.0f, 34.0f);
    DrawPanel(window, { position.x, position.y, size.x, size.y },
        10.0f, sf::Color(45, 75, 89));
    DrawText(window, text, position + sf::Vector2f(size.x * 0.5f, 17.0f),
        16, sf::Color::White, true);
}

void GameInterface::DrawPanel(sf::RenderWindow& window, sf::FloatRect rectBounds,
    float radius, sf::Color colour)
{
    const std::array<sf::Vector2f, 4> centers{
        sf::Vector2f(rectBounds.left + radius, rectBounds.top + radius),
        sf::Vector2f(rectBounds.left + rectBounds.width - radius, rectBounds.top + radius),
        sf::Vector2f(rectBounds.left + rectBounds.width - radius,
            rectBounds.top + rectBounds.height - radius),
        sf::Vector2f(rectBounds.left + radius, rectBounds.top + rectBounds.height - radius) };
    sf::ConvexShape panel(4 * (cornerSteps + 1));
    const size_t pointsPerCorner = cornerSteps + 1;
    const float angleStep = (pi * 0.5f) / static_cast<float>(cornerSteps);

    // Add a few points around each corner to round off the panel
    for (size_t corner = 0; corner < centers.size(); ++corner)
    {
        const float startAngle = -pi + static_cast<float>(corner) * pi * 0.5f;
        for (size_t point = 0; point < pointsPerCorner; ++point)
        {
            const float angle = startAngle + static_cast<float>(point) * angleStep;
            const size_t pointIndex = corner * pointsPerCorner + point;
            panel.setPoint(pointIndex,
                centers[corner] + sf::Vector2f(std::cos(angle) * radius,
                    std::sin(angle) * radius));
        }
    }
    panel.setFillColor(colour);
    window.draw(panel);
}

void GameInterface::DrawText(sf::RenderWindow& window, const std::string& text,
    sf::Vector2f position, unsigned int size, sf::Color colour,
    bool centered, float maximumWidth)
{
    sf::Text label(text, font, size);
    label.setFillColor(colour);
    const sf::FloatRect rectBounds = label.getLocalBounds();
    label.setOrigin(rectBounds.left + (centered ? rectBounds.width * 0.5f : 0.0f),
        rectBounds.top + (centered ? rectBounds.height * 0.5f : 0.0f));
    if (maximumWidth > 0.0f && rectBounds.width > maximumWidth)
    {
        const float scale = maximumWidth / rectBounds.width;
        label.setScale(scale, scale);
    }
    label.setPosition(position);
    window.draw(label);
}