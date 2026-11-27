/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Seesaw.cpp
Description : A grounded stone fulcrum and moving pipe floor that topples its load
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "Seesaw.h"

Seesaw::Seesaw(b2World& world, const sf::Texture& stoneTexture,
    const sf::Texture& pipeTexture, b2Vec2 pivot)
    : physicsWorld(world),
      support(nullptr),
      arm(nullptr),
      joint(nullptr),
      pivotCircle(pivotRadius)
{
    CreatePhysicsPieces(stoneTexture, pipeTexture, pivot);
    CreateHinge(pivot);
    SetUpPivotVisual();
}

// Make the fixed support and the pipe
void Seesaw::CreatePhysicsPieces(const sf::Texture& stoneTexture,
    const sf::Texture& pipeTexture, b2Vec2 pivot)
{
    // Extend the support below the floor so it can hold up the arm
    const float supportOffset = static_cast<float>(pipeTexture.getSize().x +
        supportPieces * stoneTexture.getSize().y) /
        (2.0f * PhysicsLibrary::pixelsPerMetre);
    const b2Vec2 supportPosition = pivot + b2Vec2(0.0f, supportOffset);
    const b2Vec2 supportSize = PhysicsLibrary::TextureSizeToMetres(
        stoneTexture.getSize(), 1, supportPieces);
    const b2Vec2 armSize = PhysicsLibrary::TextureSizeToMetres(
        pipeTexture.getSize(), 1, armPieces);

    support = std::make_unique<PhysicsObject>(physicsWorld,
        b2Shape::e_polygon, stoneTexture, supportSize, supportPosition,
        0.0f, b2_staticBody, PhysicsMaterial{}, CollisionCategory::Mechanism);
    arm = std::make_unique<PhysicsObject>(physicsWorld,
        b2Shape::e_polygon, pipeTexture, armSize, pivot,
        90.0f, b2_dynamicBody, armMaterial, CollisionCategory::Mechanism);
    arm->GetBody()->SetAngularDamping(armDamping);
}

// Connect the arm to the point and keep it locked in tipping range
void Seesaw::CreateHinge(b2Vec2 pivot)
{
    b2RevoluteJointDef hinge;
    hinge.Initialize(support->GetBody(), arm->GetBody(), pivot);
    hinge.enableLimit = true;
    hinge.lowerAngle = PhysicsLibrary::DegreesToRadians(-limitDegrees);
    hinge.upperAngle = PhysicsLibrary::DegreesToRadians(limitDegrees);
    joint = static_cast<b2RevoluteJoint*>(physicsWorld.CreateJoint(&hinge));
}

void Seesaw::SetUpPivotVisual()
{
    pivotCircle.setOrigin(pivotRadius, pivotRadius);
    pivotCircle.setFillColor(pivotColour);
}

// Remove the hinge
Seesaw::~Seesaw()
{
    physicsWorld.DestroyJoint(joint);
}

void Seesaw::Draw(sf::RenderWindow& window)
{
    support->Draw(window);
    arm->Draw(window);
    pivotCircle.setPosition(PhysicsLibrary::MetresToPixels(arm->GetBody()->GetPosition()));
    window.draw(pivotCircle);
}