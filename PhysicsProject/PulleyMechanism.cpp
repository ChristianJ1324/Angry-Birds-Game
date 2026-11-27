/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : PulleyMechanism.cpp
Description : Counterweight descent lifts one end of the enemy tray to dump its load
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "PulleyMechanism.h"

PulleyMechanism::PulleyMechanism(b2World& world, const sf::Texture& pipeTexture,
    b2Vec2 weightPosition, b2Vec2 trayPosition, float wheelHeight)
    : physicsWorld(world),
      counterweight(nullptr),
      tray(nullptr),
      joint(nullptr),
      anchorBody(nullptr),
      ropeLimit(nullptr),
      leftTrayCorner(b2Vec2_zero),
      rightTrayCorner(b2Vec2_zero),
      leftWheel(weightPosition.x, wheelHeight),
      rightWheel(trayPosition.x - attachmentOffsetX, wheelHeight),
      rope(sf::Vector2f(1.0f, ropeThickness)),
      wheel(wheelRadius)
{
    CreatePhysicsObjects(pipeTexture, weightPosition, trayPosition);
    CreatePulleyJoint(weightPosition, trayPosition);
    CreateRopeLimit(trayPosition);
    SetUpVisuals();
}

// Create the two moving pieces and save the tray corners for drawing its ropes
void PulleyMechanism::CreatePhysicsObjects(const sf::Texture& pipeTexture,
    b2Vec2 weightPosition, b2Vec2 trayPosition)
{
    const b2Vec2 weightSize = PhysicsLibrary::TextureSizeToMetres(
        pipeTexture.getSize(), weightColumns, weightRows);
    const b2Vec2 traySize = PhysicsLibrary::TextureSizeToMetres(
        pipeTexture.getSize(), 1, trayPieces);

    counterweight = std::make_unique<PhysicsObject>(physicsWorld,
        b2Shape::e_polygon, pipeTexture, weightSize, weightPosition,
        0.0f, b2_dynamicBody, weightMaterial, CollisionCategory::Mechanism);
    tray = std::make_unique<PhysicsObject>(physicsWorld,
        b2Shape::e_polygon, pipeTexture, traySize, trayPosition,
        90.0f, b2_dynamicBody, trayMaterial, CollisionCategory::Mechanism);

    counterweight->GetBody()->SetFixedRotation(true);
    tray->GetBody()->SetAngularDamping(trayDamping);
    leftTrayCorner = tray->GetBody()->GetLocalPoint(trayPosition +
        b2Vec2(-trayCornerOffsetX, -trayCornerOffsetY));
    rightTrayCorner = tray->GetBody()->GetLocalPoint(trayPosition +
        b2Vec2(trayCornerOffsetX, -trayCornerOffsetY));
}

// Tie the counterweight and tray together with the main pulley rope
void PulleyMechanism::CreatePulleyJoint(b2Vec2 weightPosition, b2Vec2 trayPosition)
{
    b2PulleyJointDef jointDef;
    jointDef.Initialize(counterweight->GetBody(), tray->GetBody(),
        leftWheel, rightWheel,
        weightPosition + weightAttachmentOffset,
        trayPosition + b2Vec2(-attachmentOffsetX, -attachmentOffsetY), pulleyRatio);
    joint = static_cast<b2PulleyJoint*>(physicsWorld.CreateJoint(&jointDef));
}

// Keep the tray attachment near the right wheel
void PulleyMechanism::CreateRopeLimit(b2Vec2 trayPosition)
{
    b2BodyDef anchorDef;
    anchorDef.position = rightWheel;
    anchorBody = physicsWorld.CreateBody(&anchorDef);
    b2DistanceJointDef limitDef;
    limitDef.Initialize(anchorBody, tray->GetBody(),
        rightWheel, trayPosition + b2Vec2(-attachmentOffsetX, -attachmentOffsetY));
    limitDef.minLength = ropeLimitMinimum;
    limitDef.maxLength = ropeLimitMaximum;
    ropeLimit = static_cast<b2DistanceJoint*>(physicsWorld.CreateJoint(&limitDef));
}

// Set the rope and pulley wheel colours and sizes
void PulleyMechanism::SetUpVisuals()
{
    rope.setFillColor(ropeColour);
    wheel.setOrigin(wheelRadius, wheelRadius);
    wheel.setFillColor(wheelColour);
    wheel.setOutlineColor(wheelOutlineColour);
    wheel.setOutlineThickness(wheelOutline);
}

// Clean up the rope joints and the anchors
PulleyMechanism::~PulleyMechanism()
{
    physicsWorld.DestroyJoint(joint);
    physicsWorld.DestroyJoint(ropeLimit);
    physicsWorld.DestroyBody(anchorBody);
}

void PulleyMechanism::Draw(sf::RenderWindow& window)
{
    DrawRope(window, leftWheel, rightWheel);
    DrawRope(window, leftWheel, joint->GetAnchorA());
    DrawRope(window, rightWheel, joint->GetAnchorB());
    DrawRope(window, joint->GetAnchorB(), tray->GetBody()->GetWorldPoint(leftTrayCorner));
    DrawRope(window, joint->GetAnchorB(), tray->GetBody()->GetWorldPoint(rightTrayCorner));
    for (const b2Vec2 wheelPosition : { leftWheel, rightWheel })
    {
        DrawRope(window, wheelPosition - b2Vec2(0.0f, suspensionStemLength), wheelPosition);
        wheel.setPosition(PhysicsLibrary::MetresToPixels(wheelPosition));
        window.draw(wheel);
    }
    counterweight->Draw(window);
    tray->Draw(window);
}

// Wake the counterweight and tray so they respond to input
void PulleyMechanism::Wake()
{
    counterweight->GetBody()->SetAwake(true);
    tray->GetBody()->SetAwake(true);
}

void PulleyMechanism::DrawRope(sf::RenderWindow& window, b2Vec2 start, b2Vec2 end)
{
    PhysicsLibrary::PositionSegment(rope, start, end, ropeThickness);
    window.draw(rope);
}