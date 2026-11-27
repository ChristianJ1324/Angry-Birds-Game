/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Slingshot.h
Description : Aims and launches birds with the sling
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once
#include "PhysicsLibrary.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

class Projectile;

class Slingshot
{
public:
    // Setup and ownership
    Slingshot(
        b2World& world,
        const sf::Texture& backgroundTexture,
        const sf::Texture& foregroundTexture,
        b2Vec2 anchorPosition);
    ~Slingshot();
    Slingshot(const Slingshot&) = delete;
    Slingshot& operator=(const Slingshot&) = delete;

    // Aiming and launch
    b2Vec2 GetLoadPosition() const { return slingAnchor + loadOffset; }
    void LoadProjectile(Projectile& projectile);
    void BeginAim(b2Vec2 mousePosition);
    bool UpdateAim(b2Vec2 mousePosition, bool hasFocus, bool mouseHeld);
    bool Release();
    void CancelAim();

    // Drawing
    void DrawBackground(sf::RenderWindow& window);
    void DrawForeground(sf::RenderWindow& window);
    void DrawAimPrediction(sf::RenderWindow& window, float groundContactY);

private:
    // Setup helpers
    void CreateAnchorBody();
    void SetUpVisuals(const sf::Texture& backgroundTexture,
        const sf::Texture& foregroundTexture);

    // Slingshot settings
    inline static const b2Vec2 loadOffset{ 0.24f, 0.0f };
    inline static const b2Vec2 backBandOffset{ -0.16f, -0.27f };
    inline static const b2Vec2 frontBandOffset{ 0.16f, -0.27f };
    inline static const float bandThickness = 4.0f;
    inline static const float spriteScale = 2.3f;
    inline static const float baseOffset = 1.18f;
    inline static const float maximumPull = 1.35f;
    inline static const float minimumLaunchPull = 0.12f;
    inline static const float launchSpeedPerMetre = 14.0f;
    inline static const float minimumLaunchSpeed = 4.0f;
    inline static const float maximumLaunchSpeed = 18.0f;
    inline static const float trajectorySampleInterval = 1.0f / 120.0f;
    inline static const float maximumTrajectorySeconds = 2.5f;
    inline static const float mouseForcePerMass = 1800.0f;
    inline static const float mouseStiffness = 140.0f;
    inline static const float mouseDamping = 16.0f;

    // Joint and trajectory helpers
    void DestroyMouseJoint();
    void DestroyDistanceJoint();
    bool CalculateLaunchVelocity(b2Vec2& launchVelocity) const;
    void DrawBand(sf::RenderWindow& window, sf::RectangleShape& band,
        b2Vec2 start, b2Vec2 end);

    // Physics state
    b2World& physicsWorld;
    b2Body* anchorBody;
    b2DistanceJoint* distanceJoint;
    b2MouseJoint* mouseJoint;
    Projectile* loadedProjectile;
    b2Vec2 slingAnchor;
    b2Vec2 backBandAnchor;
    b2Vec2 frontBandAnchor;

    sf::Sprite backgroundSprite;
    sf::Sprite foregroundSprite;
    sf::RectangleShape backSlingBand;
    sf::RectangleShape frontSlingBand;
};