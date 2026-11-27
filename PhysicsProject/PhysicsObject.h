/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : PhysicsObject.h
Description : Creates and draws one physics object
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once

#include "PhysicsLibrary.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

class PhysicsObject
{
public:
    // Body setup
    PhysicsObject(
        b2World& world,
        b2Shape::Type shapeType,
        const sf::Texture& texture,
        b2Vec2 size,
        b2Vec2 position,
        float rotation = 0.0f,
        b2BodyType bodyType = b2_dynamicBody,
        PhysicsMaterial material = {},
        CollisionCategory category = CollisionCategory::World,
        uint16_t mask = PhysicsLibrary::allCategories);
    virtual ~PhysicsObject();
    PhysicsObject(const PhysicsObject&) = delete;
    PhysicsObject& operator=(const PhysicsObject&) = delete;

    // State
    b2Body* GetBody() const { return body; }
    virtual void ReceiveImpact(float impactSpeed);
    virtual bool IsMarkedForDestruction() const { return false; }
    virtual void OnOutOfBounds() {}
    virtual void OnGroundContact() {}

    // Physics and drawing
    void ScaleSprite(float factor);
    void SetCircleRadius(float radius);
    void SetCollisionSensor(bool sensor);
    void Draw(sf::RenderWindow& window);

private:
    // Body and visual state
    b2World& physicsWorld;
    b2Body* body;
    sf::Sprite sprite;
};