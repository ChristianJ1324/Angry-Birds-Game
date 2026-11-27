/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Enemy.h
Description : Handles enemy health and floor contact
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once
#include "DamageableObject.h"

class Enemy : public DamageableObject
{
public:
    // Enemy setup and size
    Enemy(b2World& world, const sf::Texture& texture, b2Vec2 position);
    static float GetRadius() { return diameter * 0.5f; }

    // Floor contact
    void OnGroundContact() override { MarkForDestruction(); }

private:
    // Enemy physics
    inline static const float diameter = 0.56f;
    inline static const float startingHealth = 6.0f;
    inline static const float angularDamping = 0.4f;
};