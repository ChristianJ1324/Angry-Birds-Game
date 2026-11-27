/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : DamageableObject.h
Description : Adds health and damage to physics objects
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once
#include "PhysicsObject.h"

class DamageableObject : public PhysicsObject
{
public:
    // Health and damage
    DamageableObject(
        b2World& world,
        b2Shape::Type shapeType,
        const sf::Texture& texture,
        b2Vec2 size,
        b2Vec2 position,
        float startingHealth,
        CollisionCategory category = CollisionCategory::Structure,
        float rotation = 0.0f);

    void ReceiveImpact(float impactSpeed) override;
    bool IsMarkedForDestruction() const override { return markedForDestruction; }

protected:
    void MarkForDestruction()
    {
        markedForDestruction = true;
    }

private:
    void OnOutOfBounds() override;
    inline static const float minimumImpactSpeed = 1.8f;
    inline static const float impactSpeedOffset = 1.5f;
    inline static const float damagePerSpeed = 2.2f;
    inline static const PhysicsMaterial material{ 1.0f, 0.65f, 0.05f };
    float currentHealth;
    bool markedForDestruction;
};