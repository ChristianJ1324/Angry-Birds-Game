/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : MyContactListener.cpp
Description : Handles floor contacts and reports impact points
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "MyContactListener.h"
#include "PhysicsObject.h"
#include <algorithm>

// Work out how hard two objects hit and tell enemies when they land on the floor
void MyContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    const b2Manifold* manifold = contact->GetManifold();
    b2PointState oldStates[b2_maxManifoldPoints];
    b2PointState newStates[b2_maxManifoldPoints];

    // Only count contact points that started this frame
    b2GetPointStates(oldStates, newStates, oldManifold, manifold);

    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);
    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();
    float impactSpeed = 0.0f;

    for (int i = 0; i < manifold->pointCount; ++i)
    {
        if (newStates[i] == b2_addState)
        {
            const b2Vec2 point = worldManifold.points[i];

            // have to measure how quickly the bodies are closing at the actual contact point
            const b2Vec2 relativeVelocity = bodyB->GetLinearVelocityFromWorldPoint(point) -
                bodyA->GetLinearVelocityFromWorldPoint(point);

            // Use the speed towards the other body
            impactSpeed = std::max(impactSpeed, -b2Dot(relativeVelocity, worldManifold.normal));
        }
    }

    PhysicsObject* physicsObjectA = reinterpret_cast<PhysicsObject*>(bodyA->GetUserData().pointer);
    PhysicsObject* physicsObjectB = reinterpret_cast<PhysicsObject*>(bodyB->GetUserData().pointer);
    const uint16_t groundCategory = static_cast<uint16_t>(CollisionCategory::Ground);
    const bool fixtureAIsGround =
        (contact->GetFixtureA()->GetFilterData().categoryBits & groundCategory) != 0;
    const bool fixtureBIsGround =
        (contact->GetFixtureB()->GetFilterData().categoryBits & groundCategory) != 0;

    // Enemies remove themselves when they touch the ground
    if (physicsObjectA != nullptr && fixtureBIsGround)
    {
        physicsObjectA->OnGroundContact();
    }
    if (physicsObjectB != nullptr && fixtureAIsGround)
    {
        physicsObjectB->OnGroundContact();
    }

    if (impactSpeed <= 0.0f)
    {
        return;
    }

    if (physicsObjectA != nullptr)
    {
        physicsObjectA->ReceiveImpact(impactSpeed);
    }
    if (physicsObjectB != nullptr)
    {
        physicsObjectB->ReceiveImpact(impactSpeed);
    }
}