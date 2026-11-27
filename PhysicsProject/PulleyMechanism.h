/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : PulleyMechanism.h
Description : Builds the pulley, counterweight and tray
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once

#include "PhysicsObject.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>

class PulleyMechanism
{
public:
    // Setup
    PulleyMechanism(b2World& world, const sf::Texture& pipeTexture,
        b2Vec2 weightPosition, b2Vec2 trayPosition, float wheelHeight);
    ~PulleyMechanism();
    PulleyMechanism(const PulleyMechanism&) = delete;
    PulleyMechanism& operator=(const PulleyMechanism&) = delete;

    // Drawing
    void Draw(sf::RenderWindow& window);
    void Wake();

private:
    // Setup helpers
    void CreatePhysicsObjects(const sf::Texture& pipeTexture,
        b2Vec2 weightPosition, b2Vec2 trayPosition);
    void CreatePulleyJoint(b2Vec2 weightPosition, b2Vec2 trayPosition);
    void CreateRopeLimit(b2Vec2 trayPosition);
    void SetUpVisuals();

    // Drawing helpers
    void DrawRope(sf::RenderWindow& window, b2Vec2 start, b2Vec2 end);

    // Physics objects and joints
    b2World& physicsWorld;
    std::unique_ptr<PhysicsObject> counterweight;
    std::unique_ptr<PhysicsObject> tray;
    b2PulleyJoint* joint;
    b2Body* anchorBody;
    b2DistanceJoint* ropeLimit;
    b2Vec2 leftTrayCorner;
    b2Vec2 rightTrayCorner;
    b2Vec2 leftWheel;
    b2Vec2 rightWheel;

    // Rope and wheel the pulley system sits on
    sf::RectangleShape rope;
    sf::CircleShape wheel;

    // Settings
    inline static const size_t weightColumns = 3;
    inline static const size_t weightRows = 3;
    inline static const size_t trayPieces = 3;
    inline static const PhysicsMaterial weightMaterial{ 3.5f, 0.6f, 0.0f };
    inline static const PhysicsMaterial trayMaterial{ 1.0f, 0.3f, 0.0f };
    inline static const float trayDamping = 0.4f;
    inline static const float pulleyRatio = 1.0f;
    inline static const float wheelOutline = 2.0f;
    inline static const float ropeLimitMinimum = 0.4f;
    inline static const float ropeLimitMaximum = 2.5f;
    inline static const float wheelRadius = 15.0f;
    inline static const float ropeThickness = 3.0f;
    inline static const float suspensionStemLength = 0.26f;
    inline static const float attachmentOffsetX = 0.72f;
    inline static const float attachmentOffsetY = 0.37f;
    inline static const b2Vec2 weightAttachmentOffset{ 0.0f, -1.08f };
    inline static const float trayCornerOffsetX = 1.08f;
    inline static const float trayCornerOffsetY = 0.07f;
    inline static const sf::Color ropeColour{ 83, 61, 40 };
    inline static const sf::Color wheelColour{ 229, 183, 70 };
    inline static const sf::Color wheelOutlineColour{ 85, 68, 44 };
};