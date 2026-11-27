/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : PhysicsObject.cpp
Description : Creates and draws physics objects
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "PhysicsObject.h"

#include <SFML/Graphics/Texture.hpp>
#include <cmath>

// Set up the physics body and fit the sprite to it
PhysicsObject::PhysicsObject(
    b2World& world,
    b2Shape::Type shapeType,
    const sf::Texture& texture,
    b2Vec2 size,
    b2Vec2 position,
    float rotation,
    b2BodyType bodyType,
    PhysicsMaterial material,
    CollisionCategory category,
    uint16_t mask)
    : physicsWorld(world),
      body(nullptr),
      sprite(texture)
{
    const sf::Vector2u textureSize = texture.getSize();

    b2PolygonShape boxShape;
    b2CircleShape circleShape;
    b2FixtureDef fixtureDef;

    if (shapeType == b2Shape::e_polygon)
    {
        // Box2D adds a small collision skin, so keep the box inside the sprite
        const b2Vec2 halfSize(size.x * 0.5f - b2_polygonRadius,
            size.y * 0.5f - b2_polygonRadius);
        boxShape.SetAsBox(halfSize.x, halfSize.y);
        fixtureDef.shape = &boxShape;
    }
    else
    {
        circleShape.m_radius = size.x * 0.5f;
        size.y = size.x;
        fixtureDef.shape = &circleShape;
    }

    // The fixture controls how the body weighs, slides, bounces and filters collisions
    fixtureDef.density = material.density;
    fixtureDef.friction = material.friction;
    fixtureDef.restitution = material.restitution;
    fixtureDef.filter.categoryBits = static_cast<uint16_t>(category);
    fixtureDef.filter.maskBits = mask;

    // Create body at its position then attach the collision fixture
    b2BodyDef bodyDef;
    bodyDef.type = bodyType;
    bodyDef.position = position;
    bodyDef.angle = PhysicsLibrary::DegreesToRadians(rotation);

    body = physicsWorld.CreateBody(&bodyDef);
    body->CreateFixture(&fixtureDef);
    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    // have to give long pieces enough texture area to show one sprite tile per physics object
    if (shapeType == b2Shape::e_polygon)
    {
        const sf::Vector2f spriteSizePixels = PhysicsLibrary::MetresToPixels(size);
        const sf::Vector2i textureSizePixels(
            static_cast<int>(std::lround(spriteSizePixels.x)),
            static_cast<int>(std::lround(spriteSizePixels.y)));
        sprite.setTextureRect(sf::IntRect(0, 0, textureSizePixels.x, textureSizePixels.y));
        sprite.setOrigin(textureSizePixels.x * 0.5f, textureSizePixels.y * 0.5f);
    }
    else
    {
        const sf::Vector2f spriteSizePixels = PhysicsLibrary::MetresToPixels(size);
        sprite.setOrigin(static_cast<float>(textureSize.x) * 0.5f,
            static_cast<float>(textureSize.y) * 0.5f);
        sprite.setScale(spriteSizePixels.x / static_cast<float>(textureSize.x),
            spriteSizePixels.y / static_cast<float>(textureSize.y));
    }
}

// Remove the body
PhysicsObject::~PhysicsObject()
{
    if (body != nullptr)
    {
        physicsWorld.DestroyBody(body);
        body = nullptr;
    }
}

// Plain physics objects ignore hits but breakable ones handle damage themselves
void PhysicsObject::ReceiveImpact(float impactSpeed)
{
    static_cast<void>(impactSpeed);
}

// Scale the sprite without changing its physics body
void PhysicsObject::ScaleSprite(float factor)
{
    const sf::Vector2f currentScale = sprite.getScale();
    sprite.setScale(currentScale.x * factor, currentScale.y * factor);
}

void PhysicsObject::SetCircleRadius(float radius)
{
    if (body == nullptr || radius <= 0.0f)
    {
        return;
    }

    b2Fixture* fixture = body->GetFixtureList();
    if (fixture == nullptr || fixture->GetType() != b2Shape::e_circle)
    {
        return;
    }

    const b2CircleShape* currentShape =
        static_cast<const b2CircleShape*>(fixture->GetShape());
    b2CircleShape circleShape;
    circleShape.m_radius = radius;
    circleShape.m_p = currentShape->m_p;

    // Keep the existing collision settings when we replace the fixture
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = fixture->GetDensity();
    fixtureDef.friction = fixture->GetFriction();
    fixtureDef.restitution = fixture->GetRestitution();
    fixtureDef.filter = fixture->GetFilterData();
    fixtureDef.isSensor = fixture->IsSensor();
    fixtureDef.userData = fixture->GetUserData();

    body->DestroyFixture(fixture);
    body->CreateFixture(&fixtureDef);
    body->ResetMassData();
    body->SetAwake(true);
}

// Make the object solid or let other bodies pass through it
void PhysicsObject::SetCollisionSensor(bool sensor)
{
    if (body == nullptr)
    {
        return;
    }

    for (b2Fixture* fixture = body->GetFixtureList(); fixture != nullptr;
        fixture = fixture->GetNext())
    {
        fixture->SetSensor(sensor);
    }
}

void PhysicsObject::Draw(sf::RenderWindow& window)
{
    if (body == nullptr || !body->IsEnabled())
    {
        return;
    }

    sprite.setPosition(PhysicsLibrary::MetresToPixels(body->GetPosition()));
    sprite.setRotation(PhysicsLibrary::RadiansToDegrees(body->GetAngle()));
    window.draw(sprite);
}