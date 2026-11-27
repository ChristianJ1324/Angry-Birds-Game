/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Slingshot.cpp
Description : Constrains aiming with distance and mouse joints, then launches
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "Slingshot.h"
#include "Projectile.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <algorithm>
#include <cmath>

namespace
{
struct TrajectoryBounds
{
    float right;
    float bottom;
    float ground;
};

constexpr float trajectoryHalfWidth = 2.0f;
constexpr float endpointRadius = 4.0f;

bool ClipTrajectoryPoint(b2Vec2 previous, b2Vec2& current,
    const TrajectoryBounds& bounds)
{
    const b2Vec2 step = current - previous;
    float boundaryFraction = 1.0f;
    bool reachedBoundary = false;
    const auto checkBoundary = [&boundaryFraction, &reachedBoundary](
        float start, float end, float boundary)
    {
        const float fraction = (boundary - start) / (end - start);
        if (fraction >= 0.0f && fraction < boundaryFraction)
        {
            boundaryFraction = fraction;
            reachedBoundary = true;
        }
    };

    if (current.x < 0.0f && step.x < 0.0f)
    {
        checkBoundary(previous.x, current.x, 0.0f);
    }
    else if (current.x > bounds.right && step.x > 0.0f)
    {
        checkBoundary(previous.x, current.x, bounds.right);
    }
    if (current.y < 0.0f && step.y < 0.0f)
    {
        checkBoundary(previous.y, current.y, 0.0f);
    }
    else if (current.y > bounds.bottom && step.y > 0.0f)
    {
        checkBoundary(previous.y, current.y, bounds.bottom);
    }
    if (current.y >= bounds.ground && step.y > 0.0f)
    {
        checkBoundary(previous.y, current.y, bounds.ground);
    }

    if (reachedBoundary)
    {
        current = previous + boundaryFraction * step;
    }
    return reachedBoundary;
}

void AddTrajectoryDash(sf::VertexArray& dashes, b2Vec2 start, b2Vec2 end,
    sf::Color colour)
{
    const sf::Vector2f startPixels = PhysicsLibrary::MetresToPixels(start);
    const sf::Vector2f endPixels = PhysicsLibrary::MetresToPixels(end);
    const sf::Vector2f delta = endPixels - startPixels;
    const float length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (length <= 0.0f)
    {
        return;
    }

    const sf::Vector2f halfWidth(-delta.y / length * trajectoryHalfWidth,
        delta.x / length * trajectoryHalfWidth);
    dashes.append(sf::Vertex(startPixels - halfWidth, colour));
    dashes.append(sf::Vertex(startPixels + halfWidth, colour));
    dashes.append(sf::Vertex(endPixels + halfWidth, colour));
    dashes.append(sf::Vertex(endPixels - halfWidth, colour));
}
}

Slingshot::Slingshot(
    b2World& world,
    const sf::Texture& backgroundTexture,
    const sf::Texture& foregroundTexture,
    b2Vec2 anchorPosition)
    : physicsWorld(world),
      anchorBody(nullptr),
      distanceJoint(nullptr),
      mouseJoint(nullptr),
      loadedProjectile(nullptr),
      slingAnchor(anchorPosition),
      backBandAnchor(anchorPosition + backBandOffset),
      frontBandAnchor(anchorPosition + frontBandOffset),
      backgroundSprite(backgroundTexture),
      foregroundSprite(foregroundTexture),
      backSlingBand(sf::Vector2f(1.0f, bandThickness)),
      frontSlingBand(sf::Vector2f(1.0f, bandThickness))
{
    CreateAnchorBody();
    SetUpVisuals(backgroundTexture, foregroundTexture);
}

// Make a fixed point for the slingshot joints to pull against
void Slingshot::CreateAnchorBody()
{
    b2BodyDef anchorBodyDef;
    anchorBodyDef.type = b2_staticBody;
    anchorBodyDef.position = slingAnchor;
    anchorBody = physicsWorld.CreateBody(&anchorBodyDef);
}

// Place the two slingshot sprites and the rope
void Slingshot::SetUpVisuals(const sf::Texture& backgroundTexture,
    const sf::Texture& foregroundTexture)
{
    const sf::Vector2u backgroundSize = backgroundTexture.getSize();
    const sf::Vector2u foregroundSize = foregroundTexture.getSize();
    backgroundSprite.setOrigin(
        static_cast<float>(backgroundSize.x) * 0.5f,
        static_cast<float>(backgroundSize.y));
    foregroundSprite.setOrigin(
        static_cast<float>(foregroundSize.x) * 0.5f,
        static_cast<float>(foregroundSize.y));
    backgroundSprite.setScale(spriteScale, spriteScale);
    foregroundSprite.setScale(spriteScale, spriteScale);

    const b2Vec2 basePosition = slingAnchor + b2Vec2(0.0f, baseOffset);
    const sf::Vector2f basePixels = PhysicsLibrary::MetresToPixels(basePosition);
    backgroundSprite.setPosition(basePixels);
    foregroundSprite.setPosition(basePixels);
    backSlingBand.setFillColor(sf::Color(75, 42, 25));
    frontSlingBand.setFillColor(sf::Color(75, 42, 25));
}

// Remove the joints before deleting the anchor they use
Slingshot::~Slingshot()
{
    CancelAim();
    DestroyDistanceJoint();
    loadedProjectile = nullptr;
    if (anchorBody != nullptr)
    {
        physicsWorld.DestroyBody(anchorBody);
        anchorBody = nullptr;
    }
}

// Attach a bird so the player can pull it back
void Slingshot::LoadProjectile(Projectile& projectile)
{
    CancelAim();
    DestroyDistanceJoint();
    loadedProjectile = &projectile;
    const b2Vec2 startPosition = GetLoadPosition();
    projectile.GetBody()->SetTransform(startPosition, 0.0f);
    projectile.GetBody()->SetLinearVelocity(b2Vec2_zero);
    projectile.GetBody()->SetAngularVelocity(0.0f);
    projectile.GetBody()->SetGravityScale(0.0f);

    b2DistanceJointDef distanceJointDef;
    distanceJointDef.Initialize(
        anchorBody,
        projectile.GetBody(),
        slingAnchor,
        startPosition);
    distanceJointDef.minLength = b2_linearSlop;
    distanceJointDef.maxLength = maximumPull;
    distanceJoint = static_cast<b2DistanceJoint*>(physicsWorld.CreateJoint(&distanceJointDef));
}

// Start dragging wehn the click hits the bird
void Slingshot::BeginAim(b2Vec2 mousePosition)
{
    if (loadedProjectile == nullptr || distanceJoint == nullptr || mouseJoint != nullptr)
    {
        return;
    }

    bool hitProjectile = false;
    for (b2Fixture* fixture = loadedProjectile->GetBody()->GetFixtureList();
        fixture != nullptr; fixture = fixture->GetNext())
    {
        if (fixture->TestPoint(mousePosition))
        {
            hitProjectile = true;
            break;
        }
    }

    if (!hitProjectile)
    {
        return;
    }

    b2MouseJointDef mouseJointDef;
    mouseJointDef.bodyA = anchorBody;
    mouseJointDef.bodyB = loadedProjectile->GetBody();
    mouseJointDef.target = mousePosition;
    mouseJointDef.maxForce = mouseForcePerMass * mouseJointDef.bodyB->GetMass();
    mouseJointDef.stiffness = mouseStiffness;
    mouseJointDef.damping = mouseDamping;
    mouseJointDef.collideConnected = true;

    // mouse joint follows cursor - distance joint keeps bird near the slingshot
    mouseJoint = static_cast<b2MouseJoint*>(physicsWorld.CreateJoint(&mouseJointDef));
    loadedProjectile->GetBody()->SetAwake(true);
}

// Move the bird with the mouse or launch it when the let go
bool Slingshot::UpdateAim(b2Vec2 mousePosition, bool hasFocus, bool mouseHeld)
{
    if (mouseJoint == nullptr)
    {
        return false;
    }

    if (!hasFocus)
    {
        CancelAim();
        return false;
    }
    if (!mouseHeld)
    {
        return Release();
    }

    b2Vec2 pull = mousePosition - slingAnchor;
    if (pull.Length() > maximumPull)
    {
        pull.Normalize();
        mousePosition = slingAnchor + maximumPull * pull;
    }
    mouseJoint->SetTarget(mousePosition);
    return false;
}

// Launch based on how far it was pulled back
bool Slingshot::Release()
{
    if (mouseJoint == nullptr || distanceJoint == nullptr || loadedProjectile == nullptr)
    {
        CancelAim();
        return false;
    }

    b2Vec2 launchVelocity = b2Vec2_zero;

    // Read the distance before removing the joints
    const bool canLaunch = CalculateLaunchVelocity(launchVelocity);
    DestroyMouseJoint();

    if (!canLaunch)
    {
        CancelAim();
        return false;
    }

    DestroyDistanceJoint();
    Projectile* projectile = loadedProjectile;
    loadedProjectile = nullptr;
    projectile->Launch(launchVelocity);
    return true;
}

// Cancel the pull and put the bird back in the sling
void Slingshot::CancelAim()
{
    DestroyMouseJoint();
    if (loadedProjectile != nullptr)
    {
        b2Body* body = loadedProjectile->GetBody();
        body->SetTransform(GetLoadPosition(), 0.0f);
        body->SetLinearVelocity(b2Vec2_zero);
        body->SetAngularVelocity(0.0f);
    }
}

void Slingshot::DrawBackground(sf::RenderWindow& window)
{
    window.draw(backgroundSprite);

    if (loadedProjectile != nullptr && distanceJoint != nullptr)
    {
        const b2Vec2 end = loadedProjectile->GetBody()->GetPosition();
        DrawBand(window, backSlingBand, backBandAnchor, end);
    }
}

void Slingshot::DrawForeground(sf::RenderWindow& window)
{
    if (loadedProjectile != nullptr && distanceJoint != nullptr)
    {
        DrawBand(window, frontSlingBand, frontBandAnchor,
            loadedProjectile->GetBody()->GetPosition());
    }
    window.draw(foregroundSprite);
}

// Show the trajecotry while aiming
void Slingshot::DrawAimPrediction(sf::RenderWindow& window, float groundContactY)
{
    if (mouseJoint == nullptr || loadedProjectile == nullptr ||
        distanceJoint == nullptr)
    {
        return;
    }

    b2Vec2 launchVelocity = b2Vec2_zero;
    if (!CalculateLaunchVelocity(launchVelocity))
    {
        return;
    }

    const b2Vec2 start = loadedProjectile->GetBody()->GetPosition();
    const sf::Vector2u windowSize = window.getSize();
    const float worldRight = static_cast<float>(windowSize.x) /
        PhysicsLibrary::pixelsPerMetre;
    const float worldBottom = static_cast<float>(windowSize.y) /
        PhysicsLibrary::pixelsPerMetre;
    const sf::Color trajectory(128, 128, 128, 128);
    sf::VertexArray dashes(sf::Quads);
    const TrajectoryBounds bounds{ worldRight, worldBottom, groundContactY };

    b2Vec2 previous = start;
    b2Vec2 trajectoryEnd = start;
    const size_t sampleCount = static_cast<size_t>(
        maximumTrajectorySeconds / trajectorySampleInterval);
    // Sample the arc the bird follows then stop it
    for (size_t i = 1; i <= sampleCount; ++i)
    {
        const float time = static_cast<float>(i) * trajectorySampleInterval;
        b2Vec2 current = start + time * launchVelocity +
            (0.5f * time * time) * PhysicsLibrary::gravity;
        const bool reachedBoundary = ClipTrajectoryPoint(previous, current, bounds);

        // Skip every other sample to dash the line out
        if (i % 2 == 1)
        {
            AddTrajectoryDash(dashes, previous, current, trajectory);
        }

        trajectoryEnd = current;
        if (reachedBoundary)
        {
            break;
        }
        previous = current;
    }

    window.draw(dashes);
    sf::CircleShape endpointMarker(endpointRadius);
    endpointMarker.setOrigin(endpointRadius, endpointRadius);
    endpointMarker.setPosition(PhysicsLibrary::MetresToPixels(trajectoryEnd));
    endpointMarker.setFillColor(trajectory);
    window.draw(endpointMarker);
}

void Slingshot::DestroyMouseJoint()
{
    if (mouseJoint != nullptr)
    {
        physicsWorld.DestroyJoint(mouseJoint);
        mouseJoint = nullptr;
    }
}

void Slingshot::DestroyDistanceJoint()
{
    if (distanceJoint != nullptr)
    {
        physicsWorld.DestroyJoint(distanceJoint);
        distanceJoint = nullptr;
    }
}

// Work out the launch velocity for the aim line and real shot
bool Slingshot::CalculateLaunchVelocity(b2Vec2& launchVelocity) const
{
    if (loadedProjectile == nullptr || distanceJoint == nullptr)
    {
        return false;
    }

    // Use the same pull measurement for both
    b2Vec2 launchVector = GetLoadPosition() - distanceJoint->GetAnchorB();
    const float pullDistance = launchVector.Length();
    if (pullDistance < minimumLaunchPull)
    {
        return false;
    }

    launchVector.Normalize();
    const float launchSpeed = std::clamp(pullDistance * launchSpeedPerMetre,
        minimumLaunchSpeed, maximumLaunchSpeed);
    launchVelocity = launchSpeed * launchVector;
    return true;
}

void Slingshot::DrawBand(sf::RenderWindow& window, sf::RectangleShape& band,
    b2Vec2 start, b2Vec2 end)
{
    PhysicsLibrary::PositionSegment(band, start, end, bandThickness);
    window.draw(band);
}