/***********************************************************************
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) [2026] Media Design School
File Name : Game.cpp
Description : Builds scenes and runs the game
Author : Christian Jamieson
Mail : christian.jamieson@mds.ac.nz
**************************************************************************/

#include "Game.h"
#include "DamageableObject.h"
#include "Enemy.h"
#include "PhysicsObject.h"
#include "PulleyMechanism.h"
#include "Seesaw.h"
#include "Slingshot.h"

#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

// Build the game foundation
Game::Game()
    : world(PhysicsLibrary::gravity),
      gameState(GameState::LevelSelect),
      currentLevel(LevelId::None),
      nextProjectile(0),
      projectileTurn{}
{
    window.create(sf::VideoMode(windowWidth, windowHeight), "Angry Birds Physics");
    window.setFramerateLimit(frameRate);
    window.setKeyRepeatEnabled(false);
    world.SetContactListener(&contactListener);
    LoadAssets();
}

// Clear the level before removing its contact listener
Game::~Game()
{
    ClearLevel();
    world.SetContactListener(nullptr);
}

void Game::Run()
{
    sf::Clock frameClock;
    float accumulator = 0.0f;
    while (window.isOpen())
    {
        accumulator += std::min(frameClock.restart().asSeconds(), maximumFrameDelta);
        HandleEvents();
        if (!window.isOpen())
        {
            break;
        }
        while (accumulator >= physicsTimeStep)
        {
            Update(physicsTimeStep);
            accumulator -= physicsTimeStep;
        }
        Draw();
    }
}

void Game::LoadAssets()
{
    gameBackgroundTexture.loadFromFile("Sprites/game_background_1.png");
    groundTexture.loadFromFile("Sprites/ground.png");
    enemyTexture.loadFromFile("Sprites/enemy.png");
    pipeTexture.loadFromFile("Sprites/pipe.png");
    stoneTexture.loadFromFile("Sprites/stone.png");
    plankTexture.loadFromFile("Sprites/plank.png");
    slingshotBackgroundTexture.loadFromFile("Sprites/SlingshotBackground.png");
    slingshotForegroundTexture.loadFromFile("Sprites/SlingshotForeground.png");

    for (size_t i = 0; i < projectileTextures.size(); ++i)
    {
        projectileTextures[i].loadFromFile(
            Projectile::GetTexturePath(projectileSequence[i]));
    }

    groundTexture.setRepeated(true);
    pipeTexture.setRepeated(true);
    stoneTexture.setRepeated(true);
    plankTexture.setRepeated(true);

    gameBackgroundTexture.setSmooth(true);
    gameBackgroundSprite.setTexture(gameBackgroundTexture);
    const sf::Vector2u backgroundSize = gameBackgroundTexture.getSize();
    const float backgroundScale = std::max(
        static_cast<float>(windowWidth) / static_cast<float>(backgroundSize.x),
        static_cast<float>(windowHeight) / static_cast<float>(backgroundSize.y));
    gameBackgroundSprite.setOrigin(
        static_cast<float>(backgroundSize.x) * 0.5f,
        static_cast<float>(backgroundSize.y) * 0.5f);
    gameBackgroundSprite.setScale(backgroundScale, backgroundScale);
    gameBackgroundSprite.setPosition(
        static_cast<float>(windowWidth) * 0.5f,
        static_cast<float>(windowHeight) * 0.5f);

    interface.LoadFont();
}

void Game::BeginLevel(LevelId levelId)
{
    ClearLevel();
    currentLevel = levelId;
    gameState = GameState::Playing;
    CreateGround();
    slingshot = std::make_unique<Slingshot>(world,
        slingshotBackgroundTexture, slingshotForegroundTexture, b2Vec2(1.9f, 5.22f));
}

void Game::CreateSeesawLevel()
{
    BeginLevel(LevelId::Seesaw);

    // These pipe floors hold the lower enemies and support the upper section
    constexpr float lowerFloorY = 5.61f;
    PhysicsObject& leftFloor = CreatePipePlatform(
        { 6.65f, lowerFloorY }, 3, { 5.67f, 7.63f });
    CreatePipePlatform({ 9.55f, lowerFloorY }, 3, { 8.57f, 10.53f });

    const b2Vec2 seesawPosition{ 8.00f, 4.03f };
    seesaw = std::make_unique<Seesaw>(world, stoneTexture, pipeTexture,
        seesawPosition);

    // This plank props up the seesaw until a hit breaks it
    CreatePlankColumn(7.60f, 5.54f, 2, 5.0f, &leftFloor);
    constexpr float upperPlatformY = 3.15f;
    CreateWoodenPlatform({ 6.65f, upperPlatformY }, 3, { 5.97f, 7.51f }, 1, 6.0f);
    CreatePhysicsPiece(pipeTexture, b2Vec2(6.65f, 3.82f), 2, 1, 90.0f,
        b2_dynamicBody, PhysicsMaterial{ 4.0f, 0.3f, 0.0f });

    // Breaking the prop lets the tower fall onto the seesaw and flicks its right side upward
    CreateEnemyOnPlatform(6.65f, upperPlatformY, plankTexture);
    CreateEnemyOnPlatform(9.55f, seesawPosition.y, pipeTexture);
    CreateEnemyOnPlatform(6.95f, lowerFloorY, pipeTexture);
    CreateEnemyOnPlatform(9.55f, lowerFloorY, pipeTexture);
    FinishLevelSetup();
}

void Game::CreatePulleyLevel()
{
    BeginLevel(LevelId::Pulley);

    CreateStonePlatform({ 7.92f, 1.43f }, 7);
    constexpr float lowerFloorY = 5.61f;
    CreatePipePlatform({ 7.76f, lowerFloorY }, 3, { 6.78f, 8.74f });
    CreatePipePlatform({ 9.92f, lowerFloorY }, 3, { 8.94f, 10.90f });

    constexpr float roofY = 4.01f;
    std::vector<DamageableObject*> roofBeam = CreateWoodenPlatform({ 9.55f, roofY }, 4,
        { 8.21f, 10.89f }, 2, 6.0f);

    // These posts hold the roof up until the pulley tips its tray
    DamageableObject& leftRoofPiece = *roofBeam.front();
    DamageableObject& rightRoofPiece = *roofBeam.back();
    CreatePlankColumn(8.59f, 3.92f, 1, 5.0f, &leftRoofPiece);
    CreatePlankColumn(10.51f, 3.92f, 1, 5.0f, &rightRoofPiece);
    CreateWoodenPlatform({ 5.65f, 5.59f }, 2,
        { 5.03f, 6.27f }, 1, 5.0f);

    constexpr float trayY = 3.13f;
    pulley = std::make_unique<PulleyMechanism>(world, pipeTexture,
        b2Vec2(5.65f, 4.42f), b2Vec2(9.55f, trayY), 1.78f);

    CreateEnemyOnPlatform(7.76f, lowerFloorY, pipeTexture);
    CreateEnemyOnPlatform(9.92f, lowerFloorY, pipeTexture);
    CreateEnemyOnPlatform(9.55f, roofY, plankTexture);
    CreateEnemyOnPlatform(9.90f, trayY, pipeTexture);
    FinishLevelSetup();
}

// Let the level settle before loading the first bird
void Game::FinishLevelSetup()
{
    // Keep the structure still until something hits it
    for (b2Body* body = world.GetBodyList(); body != nullptr; body = body->GetNext())
    {
        if (body->GetType() == b2_dynamicBody)
        {
            body->SetAwake(false);
        }
    }
    // Load the first bird afterwards so it stays ready to aim
    LoadNextProjectile();
}

void Game::CreateGround()
{
    AddPhysicsObject(std::make_unique<PhysicsObject>(
        world, b2Shape::e_polygon, groundTexture,
        b2Vec2(worldWidth, 0.8f), b2Vec2(worldWidth * 0.5f, groundTop + 0.4f),
        0.0f, b2_staticBody, PhysicsMaterial{ 0.0f, 0.8f, 0.0f },
        CollisionCategory::Ground, PhysicsLibrary::allCategories));
}

DamageableObject& Game::CreatePlankPiece(
    b2Vec2 position, float rotation, float health)
{
    const b2Vec2 plankSize = PhysicsLibrary::TextureSizeToMetres(plankTexture.getSize());
    return AddPhysicsObject(std::make_unique<DamageableObject>(world,
        b2Shape::e_polygon, plankTexture, plankSize, position, health,
        CollisionCategory::Structure, rotation));
}

PhysicsObject& Game::CreatePhysicsPiece(const sf::Texture& texture, b2Vec2 position,
    size_t columns, size_t rows, float rotation, b2BodyType bodyType,
    PhysicsMaterial material)
{
    return AddPhysicsObject(std::make_unique<PhysicsObject>(world,
        b2Shape::e_polygon, texture,
        PhysicsLibrary::TextureSizeToMetres(texture.getSize(), columns, rows),
        position, rotation, bodyType, material, CollisionCategory::Mechanism));
}

// Join these pieces so a broken support can take the attached structure with it
void Game::ConnectStructurePieces(PhysicsObject& first, PhysicsObject& second, b2Vec2 anchor)
{
    b2WeldJointDef jointDef;
    jointDef.Initialize(first.GetBody(), second.GetBody(), anchor);
    // Box2D removes this joint when either piece is destroyed
    world.CreateJoint(&jointDef);
}

// Stack planks into a column
DamageableObject& Game::CreatePlankColumn(float positionX, float bottomY, size_t pieceCount,
    float health, PhysicsObject* foundation)
{
    const float plankLength = PhysicsLibrary::TextureSizeToMetres(plankTexture.getSize()).y;
    PhysicsObject* previousPiece = foundation;
    DamageableObject* newestPiece = nullptr;
    for (size_t i = 0; i < pieceCount; ++i)
    {
        newestPiece = &CreatePlankPiece(b2Vec2(positionX,
            bottomY - (static_cast<float>(i) + 0.5f) * plankLength),
            0.0f, health);
        if (previousPiece != nullptr)
        {
            ConnectStructurePieces(*newestPiece, *previousPiece,
                { positionX, bottomY - static_cast<float>(i) * plankLength });
        }
        previousPiece = newestPiece;
    }
    return *newestPiece;
}

// Join planks side by side to make a beam
std::vector<DamageableObject*> Game::CreatePlankBeam(b2Vec2 center, size_t pieceCount,
    float health)
{
    const float plankLength = PhysicsLibrary::TextureSizeToMetres(plankTexture.getSize()).y;
    std::vector<DamageableObject*> pieces;
    pieces.reserve(pieceCount);
    for (size_t i = 0; i < pieceCount; ++i)
    {
        const float offset = (static_cast<float>(i) -
            (static_cast<float>(pieceCount) - 1.0f) * 0.5f) * plankLength;
        DamageableObject& piece = CreatePlankPiece(center + b2Vec2(offset, 0.0f), 90.0f, health);
        if (!pieces.empty())
        {
            ConnectStructurePieces(piece, *pieces.back(),
                center + b2Vec2(offset - plankLength * 0.5f, 0.0f));
        }
        pieces.push_back(&piece);
    }
    return pieces;
}

// Build a platform with support columns
std::vector<DamageableObject*> Game::CreateWoodenPlatform(b2Vec2 center,
    size_t pieceCount, const std::vector<float>& supportXs, size_t columnPieces,
    float health)
{
    std::vector<DamageableObject*> beam = CreatePlankBeam(center, pieceCount, health);
    const b2Vec2 plankSize = PhysicsLibrary::TextureSizeToMetres(plankTexture.getSize());
    const float beamBottom = center.y + plankSize.x * 0.5f;
    const float columnBottom = beamBottom + static_cast<float>(columnPieces) * plankSize.y;
    for (float positionX : supportXs)
    {
        DamageableObject* nearestPiece = beam.front();
        for (DamageableObject* piece : beam)
        {
            if (std::abs(piece->GetBody()->GetPosition().x - positionX) <
                std::abs(nearestPiece->GetBody()->GetPosition().x - positionX))
            {
                nearestPiece = piece;
            }
        }
        // The support legs are joined to the beam, so breaking one can tip the platform
        DamageableObject& column = CreatePlankColumn(positionX, columnBottom,
            columnPieces, health);
        ConnectStructurePieces(column, *nearestPiece, { positionX, beamBottom });
    }
    return beam;
}

// Build a floor on breakable planks
PhysicsObject& Game::CreatePipePlatform(b2Vec2 center, size_t pieceCount,
    const std::vector<float>& supportXs)
{
    PhysicsObject& floor = CreatePhysicsPiece(pipeTexture, center, 1,
        pieceCount, 90.0f, b2_dynamicBody, PhysicsMaterial{ 1.5f, 0.5f, 0.0f });
    const float floorBottom = center.y +
        PhysicsLibrary::TextureSizeToMetres(pipeTexture.getSize()).x * 0.5f;
    const float columnBottom = floorBottom +
        PhysicsLibrary::TextureSizeToMetres(plankTexture.getSize()).y;
    for (float positionX : supportXs)
    {
        DamageableObject& column = CreatePlankColumn(positionX, columnBottom, 1, 6.0f);
        ConnectStructurePieces(column, floor, { positionX, floorBottom });
    }
    return floor;
}

// Add a stone platform that can't move or break
void Game::CreateStonePlatform(b2Vec2 center, size_t pieceCount)
{
    CreatePhysicsPiece(stoneTexture, center, 1, pieceCount, 90.0f,
        b2_staticBody, {});
}

void Game::CreateEnemyOnPlatform(float positionX, float platformCenterY,
    const sf::Texture& platformTexture)
{
    // A slight overlap makes sure the enemy starts in contact with the platform
    const float surfaceTop = platformCenterY -
        PhysicsLibrary::TextureSizeToMetres(platformTexture.getSize()).x * 0.5f;
    const float enemyCenterY = surfaceTop - Enemy::GetRadius() + enemyPlatformOverlap;
    Enemy& enemy = AddPhysicsObject(std::make_unique<Enemy>(world, enemyTexture,
        b2Vec2(positionX, enemyCenterY)));
    enemies.push_back(&enemy);
}

void Game::LoadNextProjectile()
{
    projectileTurn = {};
    const ProjectileType type = projectileSequence[nextProjectile++];
    Projectile& projectile = AddPhysicsObject(std::make_unique<Projectile>(world,
        projectileTextures[Projectile::GetTextureIndex(type)], type,
        slingshot->GetLoadPosition()));
    projectileTurn.projectiles.push_back(&projectile);
    slingshot->LoadProjectile(projectile);
}

// Add the shrapnel birds
void Game::AddBurstBirds(const ProjectileBurst& burst)
{
    projectileTurn.projectiles.clear();
    for (const ProjectileSpawn& spawn : burst.birds)
    {
        std::unique_ptr<Projectile> newBird = std::make_unique<Projectile>(world,
            projectileTextures[Projectile::GetTextureIndex(burst.birdType)],
            burst.birdType, spawn.position, ProjectileRole::Child);
        newBird->Launch(spawn.velocity);
        Projectile& bird = AddPhysicsObject(std::move(newBird));
        projectileTurn.projectiles.push_back(&bird);
    }
}

void Game::ClearLevel()
{
    projectileTurn = {};
    enemies.clear();
    slingshot.reset();
    seesaw.reset();
    pulley.reset();
    physicsObjects.clear();
    nextProjectile = 0;
}

void Game::RestartCurrentLevel()
{
    switch (currentLevel)
    {
    case LevelId::Seesaw:
        CreateSeesawLevel();
        break;
    case LevelId::Pulley:
        CreatePulleyLevel();
        break;
    case LevelId::None:
        ReturnToLevelSelect();
        break;
    }
}

void Game::ReturnToLevelSelect()
{
    ClearLevel();
    interface.CancelMenuPress();
    currentLevel = LevelId::None;
    gameState = GameState::LevelSelect;
}

void Game::FinishLevel(GameState finalState)
{
    slingshot->CancelAim();
    gameState = finalState;
}

void Game::HandleEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            window.close();
            return;
        case sf::Event::LostFocus:
            interface.CancelMenuPress();
            if (slingshot != nullptr)
            {
                slingshot->CancelAim();
            }
            break;
        case sf::Event::KeyPressed:
            if (window.hasFocus())
            {
                HandleKeyPress(event.key.code);
            }
            break;
        case sf::Event::MouseButtonPressed:
        case sf::Event::MouseButtonReleased:
            HandleMouseButton(event);
            break;
        default:
            break;
        }
    }
}

void Game::HandleKeyPress(sf::Keyboard::Key key)
{
    switch (gameState)
    {
    case GameState::LevelSelect:
        if (key == sf::Keyboard::Escape)
        {
            window.close();
        }
        break;
    case GameState::Playing:
    case GameState::Won:
    case GameState::Lost:
        HandleGameInput(key);
        break;
    }
}

void Game::HandleMenuEvent(const sf::Event& event)
{
    switch (interface.HandleMenuEvent(window, event))
    {
    case MenuAction::Seesaw:
        CreateSeesawLevel();
        break;
    case MenuAction::Pulley:
        CreatePulleyLevel();
        break;
    case MenuAction::Exit:
        window.close();
        break;
    case MenuAction::None:
        break;
    }
}

void Game::HandleGameInput(sf::Keyboard::Key key)
{
    switch (key)
    {
    case sf::Keyboard::Escape:
        ReturnToLevelSelect();
        break;
    case sf::Keyboard::R:
        RestartCurrentLevel();
        break;
    case sf::Keyboard::Space:
        if (gameState == GameState::Playing && projectileTurn.turnState == TurnState::Flying)
        {
            ActivateProjectile();
        }
        break;
    default:
        break;
    }
}

// Convert mouse position from screen pixels to metres
b2Vec2 Game::MousePixelsToWorld(sf::Vector2i mousePosition) const
{
    return PhysicsLibrary::PixelsToMetres(window.mapPixelToCoords(mousePosition));
}

void Game::ReleaseProjectile()
{
    if (gameState == GameState::Playing && slingshot->Release())
    {
        projectileTurn.turnState = TurnState::Flying;
    }
}

void Game::ActivateProjectile()
{
    if (projectileTurn.projectiles.empty())
    {
        return;
    }

    const ProjectileBurst projectileBurst = projectileTurn.projectiles.front()->ActivateAbility(
        MousePixelsToWorld(sf::Mouse::getPosition(window)));
    if (!projectileBurst.birds.empty())
    {
        AddBurstBirds(projectileBurst);
    }
}

    // Update the shot, step physics, then check whether the turn is over
void Game::Update(float deltaTime)
{
    if (gameState != GameState::Playing)
    {
        return;
    }

    if (slingshot->UpdateAim(MousePixelsToWorld(sf::Mouse::getPosition(window)),
        window.hasFocus(), sf::Mouse::isButtonPressed(sf::Mouse::Left)))
    {
        projectileTurn.turnState = TurnState::Flying;
    }
    for (Projectile* projectile : projectileTurn.projectiles)
    {
        projectile->UpdateAbility(deltaTime);
    }
    world.Step(deltaTime, velocityIterations, positionIterations);
    RemoveOutOfBoundsObjects();
    RemoveDestroyedObjects();
    UpdateTurn(deltaTime);
}

void Game::UpdateTurn(float deltaTime)
{
    if (enemies.empty())
    {
        FinishLevel(GameState::Won);
        return;
    }

    switch (projectileTurn.turnState)
    {
    case TurnState::Aiming:
        break;
    case TurnState::Flying:
        UpdateFlight(deltaTime);
        break;
    case TurnState::Collapse:
        UpdateCollapse(deltaTime);
        break;
    }
}

// Wait for the birds to slow down before starting the timer
void Game::UpdateFlight(float deltaTime)
{
    projectileTurn.flightSeconds += deltaTime;
    const bool allSettled = std::all_of(projectileTurn.projectiles.begin(),
        projectileTurn.projectiles.end(), [](const Projectile* projectile)
        {
            return projectile->IsTurnSettled();
        });
    // Reset timer when anything moves, level must stay settled
    projectileTurn.settledSeconds = allSettled ?
        projectileTurn.settledSeconds + deltaTime : 0.0f;

    const bool settled = projectileTurn.flightSeconds >= minimumFlightDuration &&
        projectileTurn.settledSeconds >= settledDuration;
    // End the turn if a bird keeps moving for too long
    if (projectileTurn.projectiles.empty() || settled ||
        projectileTurn.flightSeconds >= maximumFlightSeconds)
    {
        BeginCollapse();
    }
}

// Give the structure time to settle before loading the next bird
void Game::UpdateCollapse(float deltaTime)
{
    projectileTurn.collapseSeconds += deltaTime;
    if (projectileTurn.collapseSeconds >= collapseGraceSeconds)
    {
        if (nextProjectile < projectileSequence.size())
        {
            LoadNextProjectile();
        }
        else
        {
            FinishLevel(GameState::Lost);
        }
    }
}

// Start the settling delay before the next shot
void Game::BeginCollapse()
{
    projectileTurn.turnState = TurnState::Collapse;
    projectileTurn.collapseSeconds = 0.0f;
}

void Game::RemoveOutOfBoundsObjects()
{
    for (const std::unique_ptr<PhysicsObject>& physicsObject : physicsObjects)
    {
        if (!worldBounds.Contains(physicsObject->GetBody()->GetPosition()))
        {
            physicsObject->OnOutOfBounds();
        }
    }
}

// Remove destroyed objects after Box2D finishes its step
void Game::RemoveDestroyedObjects()
{
    const bool objectMarkedForRemoval = std::any_of(physicsObjects.begin(),
        physicsObjects.end(), [](const std::unique_ptr<PhysicsObject>& physicsObject)
        {
            return physicsObject->IsMarkedForDestruction();
        });

    // Wake the pulley when an object is removed so the mechanism can respond
    if (objectMarkedForRemoval && pulley != nullptr)
    {
        pulley->Wake();
    }

    // Remove observers before deleting the objects they point to
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy* enemy) { return enemy->IsMarkedForDestruction(); }), enemies.end());
    std::vector<Projectile*>& projectiles = projectileTurn.projectiles;
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const Projectile* projectile) { return projectile->IsMarkedForDestruction(); }),
        projectiles.end());
    physicsObjects.erase(std::remove_if(physicsObjects.begin(), physicsObjects.end(),
        [](const std::unique_ptr<PhysicsObject>& physicsObject)
        {
            return physicsObject->IsMarkedForDestruction();
        }), physicsObjects.end());
}

void Game::HandleMouseButton(const sf::Event& event)
{
    if (gameState == GameState::LevelSelect)
    {
        if (window.hasFocus())
        {
            HandleMenuEvent(event);
        }
        return;
    }

    if (gameState != GameState::Playing || !window.hasFocus() ||
        event.mouseButton.button != sf::Mouse::Left)
    {
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed)
    {
        slingshot->BeginAim(MousePixelsToWorld(
            { event.mouseButton.x, event.mouseButton.y }));
    }
    else
    {
        ReleaseProjectile();
    }
}

void Game::Draw()
{
    window.clear(skyColour);
    window.draw(gameBackgroundSprite);
    if (gameState == GameState::LevelSelect)
    {
        interface.DrawMenu(window, projectileTextures);
    }
    else
    {
        slingshot->DrawBackground(window);
        if (seesaw != nullptr)
        {
            seesaw->Draw(window);
        }
        if (pulley != nullptr)
        {
            pulley->Draw(window);
        }
        for (const std::unique_ptr<PhysicsObject>& physicsObject : physicsObjects)
        {
            physicsObject->Draw(window);
        }
        slingshot->DrawForeground(window);
        slingshot->DrawAimPrediction(window,
            groundTop - Projectile::GetRadius());
        DrawHud();
        if (gameState == GameState::Won || gameState == GameState::Lost)
        {
            interface.DrawResult(window, gameState == GameState::Won);
        }
    }
    window.display();
}

void Game::DrawHud()
{
    const std::string levelName = currentLevel == LevelId::Seesaw ? "Tipping Towers" : "Pulley Yard";
    const ProjectileType birdType = projectileSequence[nextProjectile - 1];
    interface.DrawHud(window, levelName, enemies.size(), nextProjectile,
        projectileSequence.size(), Projectile::GetAbilityDescription(birdType));
}