/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : DamageableObject.cpp
Description : Reduces health when objects are hit
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "DamageableObject.h"

// Creates a physics object with health and impact resistance
DamageableObject::DamageableObject(
    b2World& world,
    b2Shape::Type shapeType,
    const sf::Texture& texture,
    b2Vec2 size,
    b2Vec2 position,
    float startingHealth,
    CollisionCategory category,
    float rotation)
    : PhysicsObject(
        world,
        shapeType,
        texture,
        size,
        position,
        rotation,
        b2_dynamicBody,
        material,
        category),
      currentHealth(startingHealth),
      markedForDestruction(false)
{
}

// Reduce health when an impact is strong enough
void DamageableObject::ReceiveImpact(float impactSpeed)
{
    if (markedForDestruction || impactSpeed < minimumImpactSpeed)
    {
        return;
    }

    currentHealth -= (impactSpeed - impactSpeedOffset) * damagePerSpeed;
    if (currentHealth <= 0.0f)
    {
        MarkForDestruction();
    }
}

// Mark the object for removal if it goes off screen
void DamageableObject::OnOutOfBounds()
{
    MarkForDestruction();
}