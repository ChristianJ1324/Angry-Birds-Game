/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : PhysicsLibrary.h
Description : Handle our game physics
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once
#include "box2d/box2d.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>

enum class CollisionCategory : uint16_t
{
    World = 0x0001,
    Structure = 0x0002,
    Enemy = 0x0004,
    Projectile = 0x0008,
    Mechanism = 0x0010,
    Ground = 0x0020,
    All = World | Structure | Enemy | Projectile | Mechanism | Ground
};

struct WorldBounds
{
    float left;
    float right;
    float top;
    float bottom;

    // Check if an object is still inside the game area
    bool Contains(b2Vec2 position) const
    {
        return position.x >= left && position.x <= right &&
            position.y >= top && position.y <= bottom;
    }
};

struct PhysicsMaterial
{
    float density = 1.0f;
    float friction = 0.5f;
    float restitution = 0.1f;
};

class PhysicsLibrary
{
public:
    inline static const float pixelsPerMetre = 100.0f;
    inline static const b2Vec2 gravity{ 0.0f, 9.81f };
    inline static const uint16_t allCategories = static_cast<uint16_t>(CollisionCategory::All);

    // Convert between metres and pixels
    static sf::Vector2f MetresToPixels(b2Vec2 metres)
    {
        return { metres.x * pixelsPerMetre, metres.y * pixelsPerMetre };
    }

    static b2Vec2 PixelsToMetres(sf::Vector2f pixels)
    {
        return { pixels.x / pixelsPerMetre, pixels.y / pixelsPerMetre };
    }

    // Count repeated sprite tiles to match an object's physics size
    static b2Vec2 TextureSizeToMetres(sf::Vector2u textureSize,
        size_t columns = 1, size_t rows = 1)
    {
        return PixelsToMetres(sf::Vector2f(
            static_cast<float>(textureSize.x) * static_cast<float>(columns),
            static_cast<float>(textureSize.y) * static_cast<float>(rows)));
    }

    // Drawing helpers
    static void PositionSegment(sf::RectangleShape& segment, b2Vec2 start,
        b2Vec2 end, float thicknessPixels)
    {
        // Turn two physics points into a rotated SFML rectangle between them
        const sf::Vector2f startPixels = MetresToPixels(start);
        const sf::Vector2f endPixels = MetresToPixels(end);
        const sf::Vector2f delta = endPixels - startPixels;
        segment.setSize(sf::Vector2f(std::sqrt(delta.x * delta.x +
            delta.y * delta.y), thicknessPixels));
        segment.setOrigin(0.0f, thicknessPixels * 0.5f);
        segment.setPosition(startPixels);
        segment.setRotation(std::atan2(delta.y, delta.x) * 180.0f / b2_pi);
    }

    // Angle conversions
    static float DegreesToRadians(float degrees)
    {
        return degrees * (b2_pi / 180.0f);
    }

    static float RadiansToDegrees(float radians)
    {
        return radians * (180.0f / b2_pi);
    }
};