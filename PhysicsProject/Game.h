/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Game.h
Description : Runs the game and its scenes
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#pragma once
#include "GameInterface.h"
#include "MyContactListener.h"
#include "PhysicsLibrary.h"
#include "Projectile.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <array>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

class DamageableObject;
class Enemy;
class PhysicsObject;
class PulleyMechanism;
class Seesaw;
class Slingshot;

enum class GameState
{
    LevelSelect,
    Playing,
    Won,
    Lost
};

enum class LevelId
{
    None,
    Seesaw,
    Pulley
};

enum class TurnState
{
    Aiming,
    Flying,
    Collapse
};

struct ProjectileTurn
{
    TurnState turnState = TurnState::Aiming;
    std::vector<Projectile*> projectiles;
    float flightSeconds = 0.0f;
    float settledSeconds = 0.0f;
    float collapseSeconds = 0.0f;
};

class Game
{
public:
    // Main loop
    Game();
    ~Game();
    void Run();

private:
    // Window and physics settings
    inline static const unsigned int windowWidth = 1280;
    inline static const unsigned int windowHeight = 720;
    inline static const unsigned int frameRate = 60;
    inline static const int velocityIterations = 8;
    inline static const int positionIterations = 3;
    inline static const float physicsTimeStep = 1.0f / 60.0f;
    inline static const float maximumFrameDelta = 0.1f;

    // Levels and layout
    inline static const float enemyPlatformOverlap = 2.0f * b2_linearSlop;
    inline static const float worldWidth = 12.8f;
    inline static const float groundTop = 6.4f;
    inline static const WorldBounds worldBounds{ -0.6f, 13.4f, -2.0f, 7.6f };
    inline static const sf::Color skyColour{ 145, 205, 236 };

    //  timing and shot order
    inline static const float maximumFlightSeconds = 9.0f;
    inline static const float minimumFlightDuration = 0.8f;
    inline static const float settledDuration = 0.75f;
    inline static const float collapseGraceSeconds = 1.8f;
    inline static const std::array<ProjectileType, 3> projectileSequence{
        ProjectileType::Dive, ProjectileType::Redirect, ProjectileType::Explosion };

    // Scene setup
    void BeginLevel(LevelId levelId);
    void CreateSeesawLevel();
    void CreatePulleyLevel();
    void FinishLevelSetup();
    void LoadAssets();

    // Create and keep the physics objects used by the level
    void CreateGround();
    DamageableObject& CreatePlankPiece(b2Vec2 position, float rotation = 0.0f,
        float health = 8.0f);
    PhysicsObject& CreatePhysicsPiece(const sf::Texture& texture, b2Vec2 position,
        size_t columns, size_t rows, float rotation = 0.0f,
        b2BodyType bodyType = b2_dynamicBody, PhysicsMaterial material = {});
    template <typename ObjectType>
    ObjectType& AddPhysicsObject(std::unique_ptr<ObjectType> object)
    {
        ObjectType& addedObject = *object;
        physicsObjects.push_back(std::move(object));
        return addedObject;
    }

    // Build the platforms and supports from those pieces
    void ConnectStructurePieces(PhysicsObject& first, PhysicsObject& second, b2Vec2 anchor);
    DamageableObject& CreatePlankColumn(float positionX, float bottomY, size_t pieceCount,
        float health = 8.0f, PhysicsObject* foundation = nullptr);
    std::vector<DamageableObject*> CreatePlankBeam(b2Vec2 center, size_t pieceCount,
        float health = 8.0f);
    std::vector<DamageableObject*> CreateWoodenPlatform(b2Vec2 center,
        size_t pieceCount, const std::vector<float>& supportXs, size_t columnPieces,
        float health = 8.0f);
    PhysicsObject& CreatePipePlatform(b2Vec2 center, size_t pieceCount,
        const std::vector<float>& supportXs);

    void CreateStonePlatform(b2Vec2 center, size_t pieceCount);
    void CreateEnemyOnPlatform(float positionX, float platformCenterY,
        const sf::Texture& platformTexture);

    // Level and turns
    void LoadNextProjectile();
    void AddBurstBirds(const ProjectileBurst& burst);
    void ClearLevel();
    void RestartCurrentLevel();
    void ReturnToLevelSelect();
    void FinishLevel(GameState finalState);

    // Input functions
    void HandleEvents();
    void HandleKeyPress(sf::Keyboard::Key key);
    void HandleMenuEvent(const sf::Event& event);
    void HandleGameInput(sf::Keyboard::Key key);
    b2Vec2 MousePixelsToWorld(sf::Vector2i mousePosition) const;
    void ReleaseProjectile();
    void ActivateProjectile();
    void HandleMouseButton(const sf::Event& event);

    // Update functions
    void Update(float deltaTime);
    void UpdateTurn(float deltaTime);
    void UpdateFlight(float deltaTime);
    void UpdateCollapse(float deltaTime);
    void BeginCollapse();
    void RemoveOutOfBoundsObjects();
    void RemoveDestroyedObjects();

    // Drawing functions
    void Draw();
    void DrawHud();

    // Window and physics
    sf::RenderWindow window;
    b2World world;
    MyContactListener contactListener;

    // Textures and interface
    sf::Texture gameBackgroundTexture;
    sf::Sprite gameBackgroundSprite;
    sf::Texture groundTexture;
    sf::Texture enemyTexture;
    std::array<sf::Texture, 3> projectileTextures;
    sf::Texture pipeTexture;
    sf::Texture stoneTexture;
    sf::Texture plankTexture;
    sf::Texture slingshotBackgroundTexture;
    sf::Texture slingshotForegroundTexture;
    GameInterface interface;

    // Level objects
    std::unique_ptr<Slingshot> slingshot;
    std::unique_ptr<Seesaw> seesaw;
    std::unique_ptr<PulleyMechanism> pulley;
    std::vector<std::unique_ptr<PhysicsObject>> physicsObjects;
    std::vector<Enemy*> enemies;

    // Game states
    GameState gameState;
    LevelId currentLevel;
    size_t nextProjectile;
    ProjectileTurn projectileTurn;
};