/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Projectile.h
Description : Bird physics and abilities
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once
#include "PhysicsObject.h"
#include <array>
#include <cstddef>
#include <vector>

enum class ProjectileType
{
    Dive,
    Redirect,
    Explosion
};

struct ProjectileSpawn
{
    b2Vec2 position{ 0.0f, 0.0f };
    b2Vec2 velocity{ 0.0f, 0.0f };
};

// Keep track of the shrapnel
struct ProjectileBurst
{
    ProjectileType birdType = ProjectileType::Dive;
    std::vector<ProjectileSpawn> birds;
};

enum class ProjectileState
{
    Loaded,
    Flying,
    Retired
};

enum class ProjectileRole
{
    Parent,
    Child
};

class Projectile : public PhysicsObject
{
public:
    // Setup and bird info
    Projectile(b2World& world, const sf::Texture& texture, ProjectileType type,
        b2Vec2 position, ProjectileRole role = ProjectileRole::Parent);
    static const char* GetAbilityDescription(ProjectileType type);
    static const char* GetTexturePath(ProjectileType type);
    static size_t GetTextureIndex(ProjectileType type);
    static float GetRadius() { return parentDiameter * 0.5f; }

    // Flight and abilities
    void Launch(b2Vec2 velocity);
    ProjectileBurst ActivateAbility(b2Vec2 target);
    void UpdateAbility(float deltaTime);

    // Bird state
    bool IsMarkedForDestruction() const override { return flightState == ProjectileState::Retired; }
    void OnOutOfBounds() override;
    bool IsTurnSettled() const;

private:
    // Bird settings
    inline static const std::array<const char*, 3> texturePaths{
        "Sprites/parrot.png", "Sprites/chick.png", "Sprites/duck.png" };
    inline static const uint16_t collisionMask = PhysicsLibrary::allCategories &
        ~static_cast<uint16_t>(CollisionCategory::Projectile);
    inline static const float parentDiameter = 0.52f;
    inline static const float childDiameter = 0.22f;
    inline static const float parentDensity = 1.7f;
    inline static const float childDensity = 1.8f;
    inline static const float friction = 0.65f;
    inline static const float restitution = 0.08f;
    inline static const float angularDamping = 0.7f;
    inline static const float minimumTargetDistance = 0.1f;
    inline static const float settledSpeedSquared = 0.25f;
    inline static const float settledAngularSpeed = 0.8f;
    inline static const size_t burstBirdCount = 12;
    inline static const float redirectSpeed = 17.0f;
    inline static const float burstSpeed = 14.0f;
    inline static const float childSpawnRadius = 0.44f;
    inline static const float diveScale = 2.0f;
    inline static const float diveGravityScale = 2.0f;
    inline static const float growthDuration = 0.5f;

    // Ability helpers
    ProjectileBurst CreateBurst();
    void BeginGrowthDive();
    void Redirect(b2Vec2 target);
    void Retire();

    // Bird state
    ProjectileType birdType;
    const ProjectileRole birdRole;
    bool abilityUsed;
    bool growing;
    float growthElapsed;
    float currentScale;
    ProjectileState flightState;
};