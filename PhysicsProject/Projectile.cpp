/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Projectile.cpp
Description : Handles bird flight and abilities
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "Projectile.h"
#include <algorithm>
#include <cmath>

// Set up a bird
Projectile::Projectile(b2World& world, const sf::Texture& texture,
    ProjectileType type, b2Vec2 position, ProjectileRole role)
    : PhysicsObject(world, b2Shape::e_circle, texture,
        role == ProjectileRole::Child ? b2Vec2(childDiameter, childDiameter) :
            b2Vec2(parentDiameter, parentDiameter),
        position, 0.0f, b2_dynamicBody,
        PhysicsMaterial{ role == ProjectileRole::Child ? childDensity : parentDensity,
            friction, restitution }, CollisionCategory::Projectile, collisionMask),
      birdType(type),
      birdRole(role),
      abilityUsed(role == ProjectileRole::Child),
      growing(false),
      growthElapsed(0.0f),
      currentScale(1.0f),
      flightState(ProjectileState::Loaded)
{
    GetBody()->SetBullet(true);
    GetBody()->SetAngularDamping(angularDamping);
}

const char* Projectile::GetAbilityDescription(ProjectileType type)
{
    switch (type)
    {
    case ProjectileType::Dive:
        return "Parrot - Press space to grow and dive";
    case ProjectileType::Redirect:
        return "Chick - Press space to launch towards the cursor";
    case ProjectileType::Explosion:
        return "Duck - Press space to explode and send shrapnel in all directions";
    }
    return "";
}

const char* Projectile::GetTexturePath(ProjectileType type)
{
    const size_t index = GetTextureIndex(type);
    return index < texturePaths.size() ? texturePaths[index] : "";
}

size_t Projectile::GetTextureIndex(ProjectileType type)
{
    switch (type)
    {
    case ProjectileType::Dive:
        return 0;
    case ProjectileType::Redirect:
        return 1;
    case ProjectileType::Explosion:
        return 2;
    }
    return texturePaths.size();
}

void Projectile::OnOutOfBounds()
{
    if (flightState == ProjectileState::Flying)
    {
        Retire();
    }
}

// Give the bird its launch speed and restore gravity
void Projectile::Launch(b2Vec2 velocity)
{
    if (flightState != ProjectileState::Loaded)
    {
        return;
    }

    flightState = ProjectileState::Flying;
    GetBody()->SetGravityScale(1.0f);
    GetBody()->SetLinearVelocity(b2Vec2_zero);
    GetBody()->ApplyLinearImpulseToCenter(GetBody()->GetMass() * velocity, true);
}

// Handle ability input while parent bird is flying
ProjectileBurst Projectile::ActivateAbility(b2Vec2 target)
{
    if (flightState != ProjectileState::Flying || birdRole == ProjectileRole::Child || abilityUsed)
    {
        return {};
    }

    switch (birdType)
    {
    case ProjectileType::Dive:
        abilityUsed = true;
        BeginGrowthDive();
        break;

    case ProjectileType::Redirect:
        Redirect(target);
        break;

    case ProjectileType::Explosion:
        abilityUsed = true;
        return CreateBurst();
    }
    return {};
}

// Grow the parrot first, then make it solid and send it diving
void Projectile::UpdateAbility(float deltaTime)
{
    if (!growing)
    {
        return;
    }

    growthElapsed = std::min(growthElapsed + deltaTime, growthDuration);
    const float progress = growthElapsed / growthDuration;
    const float targetScale = 1.0f + (diveScale - 1.0f) * progress;
    // ScaleSprite multiplies the current size, so have to apply only the change since last frame
    ScaleSprite(targetScale / currentScale);
    currentScale = targetScale;

    if (progress < 1.0f)
    {
        return;
    }

    // Match the collider to the finished sprite before letting the parrot slam down
    SetCircleRadius(GetRadius() * diveScale);
    SetCollisionSensor(false);
    GetBody()->SetGravityScale(diveGravityScale);
    GetBody()->SetLinearVelocity(b2Vec2_zero);
    GetBody()->SetAngularVelocity(0.0f);
    GetBody()->SetAwake(true);
    growing = false;
}

bool Projectile::IsTurnSettled() const
{
    return flightState == ProjectileState::Retired || (!growing &&
        (GetBody()->GetLinearVelocity().LengthSquared() < settledSpeedSquared &&
            std::abs(GetBody()->GetAngularVelocity()) < settledAngularSpeed));
}

// Split the duck into shrapenl and remove the original
ProjectileBurst Projectile::CreateBurst()
{
    ProjectileBurst burst;
    burst.birdType = birdType;
    burst.birds.reserve(burstBirdCount);
    const float angleStep = 2.0f * b2_pi / static_cast<float>(burstBirdCount);
    for (size_t i = 0; i < burstBirdCount; ++i)
    {
        // Each bird gets its own point on the circle and flies straight outward
        const float angle = angleStep * static_cast<float>(i);
        const b2Vec2 direction(std::cos(angle), std::sin(angle));
        ProjectileSpawn birdSpawn;
        birdSpawn.position = GetBody()->GetPosition() + childSpawnRadius * direction;
        birdSpawn.velocity = burstSpeed * direction;
        burst.birds.push_back(birdSpawn);
    }

    Retire();
    return burst;
}

// Freeze bird in mid air and scale it up
void Projectile::BeginGrowthDive()
{
    growing = true;
    growthElapsed = 0.0f;
    currentScale = 1.0f;
    GetBody()->SetLinearVelocity(b2Vec2_zero);
    GetBody()->SetAngularVelocity(0.0f);
    GetBody()->SetGravityScale(0.0f);
    GetBody()->SetAwake(true);
    SetCollisionSensor(true);
}

// Point towards the cursor
void Projectile::Redirect(b2Vec2 target)
{
    b2Vec2 direction = target - GetBody()->GetPosition();
    if (direction.Normalize() < minimumTargetDistance)
    {
        return;
    }

    abilityUsed = true;
    GetBody()->SetLinearVelocity(redirectSpeed * direction);
    GetBody()->SetAwake(true);
}

// Turn off the bird until its body is removed
void Projectile::Retire()
{
    flightState = ProjectileState::Retired;
    GetBody()->SetEnabled(false);
}