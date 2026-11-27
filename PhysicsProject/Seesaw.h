/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Seesaw.h
Description : Builds the hinged pipe floor in the first scene
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once

#include "PhysicsObject.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>

class Seesaw
{
public:
    // Setup
    Seesaw(b2World& world, const sf::Texture& stoneTexture,
        const sf::Texture& pipeTexture, b2Vec2 pivot);
    ~Seesaw();
    Seesaw(const Seesaw&) = delete;
    Seesaw& operator=(const Seesaw&) = delete;

    // Drawing
    void Draw(sf::RenderWindow& window);

private:
    // Setup helpers
    void CreatePhysicsPieces(const sf::Texture& stoneTexture,
        const sf::Texture& pipeTexture, b2Vec2 pivot);
    void CreateHinge(b2Vec2 pivot);
    void SetUpPivotVisual();

    // Physics pieces and hinge
    b2World& physicsWorld;
    std::unique_ptr<PhysicsObject> support;
    std::unique_ptr<PhysicsObject> arm;
    b2RevoluteJoint* joint;

    // Pivot point
    sf::CircleShape pivotCircle;

    // Seesaw settings
    inline static const size_t armPieces = 6;
    inline static const size_t supportPieces = 4;
    inline static const PhysicsMaterial armMaterial{ 1.0f, 0.35f, 0.0f };
    inline static const float armDamping = 0.3f;
    inline static const float limitDegrees = 50.0f;
    inline static const float pivotRadius = 6.0f;
    inline static const sf::Color pivotColour{ 65, 72, 77 };
};