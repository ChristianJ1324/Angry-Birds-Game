/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Enemy.cpp
Description : Creates the enemy body
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "Enemy.h"

Enemy::Enemy(b2World& world, const sf::Texture& texture, b2Vec2 position)
    : DamageableObject(
        world,
        b2Shape::e_circle,
        texture,
        b2Vec2(diameter, diameter),
        position,
        startingHealth,
        CollisionCategory::Enemy)
{
    GetBody()->SetAngularDamping(angularDamping);
}