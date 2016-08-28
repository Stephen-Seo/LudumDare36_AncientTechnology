
#include "GameScreen.hpp"

#include <cmath>
#include <list>

#include <engine/resourceManager.hpp>
#include <engine/utility.hpp>

#include <EC/Meta/Meta.hpp>

#include "components/Components.hpp"
#include "GameContext.hpp"

#ifndef NDEBUG
 #include <iostream>
 #include <ios>
#endif

const float GameScreen::playerEdges[16] = {
    -16.0f, -32.0f,
    0.0f, -32.0f,
    16.0f, -32.0f,
    16.0f, 0.0f,
    16.0f, 32.0f,
    0.0f, 32.0f,
    -16.0f, 32.0f,
    -16.0f, 0.0f
};

GameScreen::GameScreen(StateStack& stack, Context context) :
State(stack, context),
flags(0),
rdist(),
garbageTimer(0),
drawFrameTimer(0),
timer(0),
shipTimer{0, 0},
shipPos(480.0f, 480.0f),
playerHP(GAME_MAX_PLAYER_HP),
playerInvisTime(0.0f),
playerRegenTimer(0),
asteroidHP(GAME_ASTEROID_PHASE_0_HP),
asteroidPhase(0),
asteroidExplosionsTimer(0.0f),
asteroidExplosionsCount(0),
screenFlashTimer(0),
asteroidFireTimer(0),
asteroidRotationAngle(0)
{
    context.resourceManager->registerTexture(*this, "res/Planet.png");
    context.resourceManager->registerTexture(*this, "res/ship.png");
    context.resourceManager->registerTexture(*this, "res/volumeButton.png");
    context.resourceManager->registerTexture(*this, "res/healthBar.png");
    context.resourceManager->registerTexture(*this, "res/asteroidSymbol.png");
    context.resourceManager->loadResources();

    sf::Texture& planetTexture = context.resourceManager->getTexture("res/Planet.png");
    sf::Texture& shipTexture = context.resourceManager->getTexture("res/ship.png");
    sf::Texture& healthBarTexture = context.resourceManager->getTexture("res/healthBar.png");
    asteroidSymbolTexture = &context.resourceManager->getTexture("res/asteroidSymbol.png");

    planetRect.setSize(sf::Vector2f(1024.0f, 1024.0f));
    planetRect.setTexture(&planetTexture, true);
    planetRect.setPosition((1024.0f - 960.0f) / -2.0f, 540.0f - 192.0f);

    for(unsigned int i = 0; i < 2; ++i)
    {
        ship[i].setTexture(shipTexture);
        ship[i].setTextureRect(sf::IntRect(i * 64, 0, 64, 160));
        ship[i].setScale(0.5f, 0.5f);
        ship[i].setOrigin(32.0f, 64.0f);
        ship[i].setPosition(480.0f, 540.0f);
    }
    ship[1].setColor(sf::Color(127, 55, 255));

    sf::Texture& volumeTexture = context.resourceManager->getTexture("res/volumeButton.png");

    volumeButton.setTexture(volumeTexture);
    volumeButton.setTextureRect(sf::IntRect(0, 0, 64, 64));

    for(unsigned int i = 0; i < 2; ++i)
    {
        healthBar[i].setTexture(healthBarTexture);
        healthBar[i].setTextureRect(sf::IntRect(i * 64, 0, 64, 160));
        healthBar[i].setColor(sf::Color(255, 255, 255, 127));
    }

    GameContext* gc = static_cast<GameContext*>(context.extraContext);

    auto eid = gc->gameManager.addEntity();
    gc->gameManager.addComponent<Position>(eid, 480.0f, 135.0f);
    gc->gameManager.addComponent<Velocity>(eid);
    gc->gameManager.addComponent<Acceleration>(eid);
    gc->gameManager.addComponent<Rotation>(eid);
    gc->gameManager.addComponent<AngularVelocity>(eid, 70.0f);
    gc->gameManager.addComponent<Offset>(eid, 64.0f, 64.0f);
    gc->gameManager.addComponent<Size>(eid, 128.0f, 128.0f);
    gc->gameManager.addTag<TAsteroid>(eid);
    asteroidID = eid;

    context.window->setFramerateLimit(60);

    view = context.window->getView();
    view.setCenter(sf::Vector2f(view.getSize().x / 2.0f, 405.0f + view.getSize().y / 2.0f));
    context.window->setView(view);

    bgMusic.openFromFile("res/LudumDare36_SpaceFight.ogg");
    bgMusic.setLoop(true);
    bgMusic.play();
}

GameScreen::~GameScreen()
{
}

using EntitySignature = EC::Meta::TypeList<
    Position,
    Velocity,
    Acceleration,
    Rotation,
    AngularVelocity,
    Offset,
    Size
>;

void GameScreen::draw(Context context)
{
    context.window->draw(planetRect);

    for(unsigned int i = 0; i < 2; ++i)
    {
        context.window->draw(ship[i]);
    }

    GameContext* gc = static_cast<GameContext*>(context.extraContext);

    std::list<sf::CircleShape> explosionsList;

    gc->gameManager.forMatchingSignature<EntitySignature>([this, &context, &gc, &explosionsList]
       (std::size_t eid,
        Position& pos,
        Velocity& vel,
        Acceleration& acc,
        Rotation& rotation,
        AngularVelocity& vrotation,
        Offset& offset,
        Size& size) {
        if(gc->gameManager.hasTag<TAsteroid>(eid))
        {
            return;
        }
        else if(gc->gameManager.hasTag<TParticle>(eid))
        {
            Timer& timer = gc->gameManager.getEntityData<Timer>(eid);
            float r = rdist(*context.randomEngine);
            this->drawRect.setFillColor(sf::Color(
                (unsigned char)(r * 100.0f + 100.0f),
                (unsigned char)(r * 35.0f + 35.0f),
                (unsigned char)(r * 35.0f + 35.0f),
                (unsigned char)((1.0f - timer.time / GAME_ASTEROID_PARTICLE_LIFETIME) * 255.0f)
            ));
        }
        else if(gc->gameManager.hasTag<TAsteroidProjectile>(eid))
        {
            this->drawRect.setFillColor(sf::Color::Red);
        }
        else
        {
            this->drawRect.setFillColor(sf::Color::White);
        }

        if(!gc->gameManager.hasTag<TExplosion>(eid))
        {
            this->drawRect.setSize(sf::Vector2f(size.width, size.height));
            this->drawRect.setPosition(pos.x, pos.y);
            this->drawRect.setOrigin(offset.x, offset.y);
            this->drawRect.setRotation(rotation.rotation);

            context.window->draw(this->drawRect);
        }
        else
        {
            Timer& timer = gc->gameManager.getEntityData<Timer>(eid);

            float growth = 64.0f * timer.time / GAME_EXPLOSION_LIFETIME;
            this->drawCircle.setRadius(size.width / 2.0f + growth);
            this->drawCircle.setPosition(pos.x, pos.y);
            this->drawCircle.setOrigin(size.width / 2.0f + growth, size.width / 2.0f + growth);

            int value = (timer.time / GAME_EXPLOSION_LIFETIME) * 255.0f;
            int alpha = (timer.time < GAME_EXPLOSION_LIFETIME / 2.0f ? 255 : 255.0f * (1.0f - (timer.time - GAME_EXPLOSION_LIFETIME / 2.0f) / (GAME_EXPLOSION_LIFETIME / 2.0F)));
            this->drawCircle.setFillColor(sf::Color(
                255,
                value,
                value,
                alpha));

//                context.window->draw(this->drawCircle);
            explosionsList.push_back(drawCircle);
        }
    });

    // draw asteroid
    Position& aPos = gc->gameManager.getEntityData<Position>(asteroidID);
    Rotation& aRot = gc->gameManager.getEntityData<Rotation>(asteroidID);
    Size& aSize = gc->gameManager.getEntityData<Size>(asteroidID);
    Offset& aOffset = gc->gameManager.getEntityData<Offset>(asteroidID);

    drawRect.setSize(sf::Vector2f(aSize.width, aSize.height));
    drawRect.setPosition(aPos.x, aPos.y);
    drawRect.setOrigin(aOffset.x, aOffset.y);
    drawRect.setRotation(aRot.rotation);

    if(asteroidPhase <= 5 && drawFrameTimer % 10 == 0)
    {
        float r = rdist(*context.randomEngine);
        asteroidColor = sf::Color(
            (unsigned char)(r * 100.0f + 100.0f),
            (unsigned char)(r * 35.0f + 35.0f),
            (unsigned char)(r * 35.0f + 35.0f)
        );
    }
    else if(asteroidPhase == 6)
    {
        asteroidColor = sf::Color(
            asteroidColor.r,
            asteroidColor.g,
            asteroidColor.b,
            255.0f * (1.0f - timer / GAME_ASTEROID_DEATH_TIME)
        );
    }

    if(asteroidPhase <= 6)
    {
        drawRect.setFillColor(asteroidColor);
        context.window->draw(drawRect);
    }

    // draw asteroid symbol
    drawRect.setTexture(asteroidSymbolTexture, true);
    if(asteroidPhase == 0)
    {
        int alpha = (int)(255.0f * (1.0f - (float)asteroidHP / (float)GAME_ASTEROID_PHASE_0_HP));
        drawRect.setFillColor(sf::Color(255, 255, 255, alpha));
    }
    else if(asteroidPhase <= 5)
    {
        drawRect.setFillColor(sf::Color::White);
    }
    else if(asteroidPhase == 6)
    {
        drawRect.setFillColor(sf::Color(255, 255, 255, 255.0f * (1.0f - timer / GAME_ASTEROID_DEATH_TIME)));
    }

    if(asteroidPhase <= 6)
    {
        context.window->draw(drawRect);
    }
    drawRect.setTexture(nullptr);

    // draw explosions
    for(auto explosion : explosionsList)
    {
        context.window->draw(explosion);
    }

    if((flags & 0x100) == 0)
    {
        volumeButton.setTextureRect(sf::IntRect(0, 0, 64, 64));
        context.window->draw(volumeButton);
    }
    else
    {
        volumeButton.setTextureRect(sf::IntRect(64, 0, 64, 64));
        context.window->draw(volumeButton);
    }

    context.window->draw(healthBar[0]);
    context.window->draw(healthBar[1]);

    // draw screen flash
    if((flags & 0x200) != 0)
    {
        drawRect.setSize(context.window->getView().getSize());
        drawRect.setPosition(context.window->getView().getCenter() - drawRect.getSize() / 2.0f);
        drawRect.setFillColor(sf::Color(255, 255, 255, (int)(255.0f * screenFlashTimer / GAME_SCREEN_FLASH_FAST_TIME)));
        drawRect.setOrigin(0, 0);
        drawRect.setRotation(0);
        context.window->draw(drawRect);
    }
    else if((flags & 0x400) != 0)
    {
        drawRect.setSize(context.window->getView().getSize());
        drawRect.setPosition(context.window->getView().getCenter() - drawRect.getSize() / 2.0f);
        drawRect.setFillColor(sf::Color(255, 255, 255, (int)(255.0f * screenFlashTimer / GAME_SCREEN_FLASH_SLOW_TIME)));
        drawRect.setOrigin(0, 0);
        drawRect.setRotation(0);
        context.window->draw(drawRect);
    }

    if(++drawFrameTimer >= 60)
    {
        drawFrameTimer -= 60;
    }
}

bool GameScreen::update(sf::Time dt, Context context)
{
    GameContext* gc = static_cast<GameContext*>(context.extraContext);

    timer += dt.asSeconds();
    if((flags & 0x1) == 0)
    {
        // pan to asteroid
        if(timer <= 5.0f)
        {
            float amount = timer / 5.0f;
            view.setCenter(sf::Vector2f(view.getSize().x / 2.0f, 405.0f * (1.0f - amount * amount) + view.getSize().y / 2.0f));
            context.window->setView(view);
        }
        else
        {
            timer = 0.0f;
            flags |= 0x1;
            view.setCenter(sf::Vector2f(view.getSize().x / 2.0f, view.getSize().y / 2.0f));
            context.window->setView(view);
        }
    }
    else if(asteroidPhase == 6)
    {
        if(timer >= GAME_ASTEROID_DEATH_TIME)
        {
            asteroidPhase = 7;
            timer = 0;
            gc->gameManager.deleteEntity(asteroidID);
            bgMusic.openFromFile("res/LudumDare36_Peace.ogg");
            bgMusic.setLoop(true);
            if((flags & 0x100) == 0)
            {
                bgMusic.play();
            }
        }
    }

    playerInvisTime -= dt.asSeconds();
    if(playerInvisTime < 0.0f)
    {
        playerInvisTime = 0.0f;
        if(playerHP > 0)
        {
            ship[0].setColor(sf::Color::White);
        }
    }

    playerRegenTimer += dt.asSeconds();
    if(playerRegenTimer >= GAME_PLAYER_REGEN_TIME)
    {
        playerRegenTimer = 0;
        playerHP += 5;
        if(playerHP > GAME_MAX_PLAYER_HP)
        {
            playerHP = GAME_MAX_PLAYER_HP;
        }
    }

    if((flags & 0x200) != 0 || (flags & 0x400) != 0)
    {
        screenFlashTimer += dt.asSeconds();
        if(((flags & 0x200) != 0 && screenFlashTimer >= GAME_SCREEN_FLASH_FAST_TIME) ||
            ((flags & 0x400) != 0 && screenFlashTimer >= GAME_SCREEN_FLASH_SLOW_TIME))
        {
            screenFlashTimer = 0;
            flags &= 0xFFFFFFFFFFFFF9FF;
        }
    }

    updateHealthBar(context);

    checkAsteroidExplosions(dt, context);

    fireTimer -= dt.asSeconds();
    if(fireTimer < 0)
    {
        fireTimer = 0.0f;
    }

    playerInput(dt, context);

    sf::Vector2f updatedPos = shipPos + view.getCenter() - view.getSize() / 2.0f;
    for(unsigned int i = 0; i < 2; ++i)
    {
        ship[i].setPosition(updatedPos);
    }

    animateShipThruster(dt);

    // aim ship at mouse
    sf::Vector2f v = context.window->mapPixelToCoords(mousePos);
    v = v - ship[0].getPosition();
    float angle = std::atan2(v.y, v.x) + std::acos(-1) / 2.0f;
    for(unsigned int i = 0; i < 2; ++i)
    {
        ship[i].setRotation(angle * 180 / std::acos(-1));
    }


    Position asteroidPos;

    gc->gameManager.forMatchingSignature<EntitySignature>([this, &gc, &dt, &asteroidPos, &context]
       (std::size_t eid,
        Position& pos,
        Velocity& vel,
        Acceleration& acc,
        Rotation& rotation,
        AngularVelocity& vrotation,
        Offset& offset,
        Size& size)
    {
        vel.x += acc.x * dt.asSeconds();
        vel.y += acc.y * dt.asSeconds();

        pos.x += vel.x * dt.asSeconds();
        pos.y += vel.y * dt.asSeconds();

        rotation.rotation += vrotation.vrotation * dt.asSeconds();

        if(rotation.rotation >= 360.0f)
        {
            rotation.rotation -= 360.0f;
        }
        else if(rotation.rotation < 0)
        {
            rotation.rotation += 360.0f;
        }

        if(asteroidPhase == 4 && gc->gameManager.hasTag<TAsteroid>(eid))
        {
            if(pos.y > 270.0f)
            {
                pos.y = 270.0f;
                vel.y = 0;
            }
        }

        if(gc->gameManager.hasTag<TAsteroid>(eid) || gc->gameManager.hasTag<TAsteroidProjectile>(eid))
        {

            // collision detection between player and asteroid (projectiles)
            // x = x * cos + y * -sin
            // y = x * sin + y * cos
            // [  cos -sin ]
            // [  sin  cos ]
            // ??? probably if memory serves me correctly
            // TODO fix

            sf::Transform transform;
            transform.rotate(rotation.rotation, pos.x, pos.y);
            sf::VertexArray coords(sf::PrimitiveType::Points, 4);
            coords[0].position = transform * sf::Vector2f(
                pos.x - size.width / 2.0f,
                pos.y - size.height / 2.0f);
            coords[1].position = transform * sf::Vector2f(
                pos.x + size.width / 2.0f,
                pos.y - size.height / 2.0f);
            coords[2].position = transform * sf::Vector2f(
                pos.x + size.width / 2.0f,
                pos.y + size.height / 2.0f);
            coords[3].position = transform * sf::Vector2f(
                pos.x - size.width / 2.0f,
                pos.y + size.height / 2.0f);

            transform = sf::Transform::Identity;
            transform.rotate(this->ship[0].getRotation(), this->ship[0].getPosition());
            sf::Vector2f shipEdge;
            for(unsigned int i = 0; i < 16; i += 2)
            {
                shipEdge = transform * (this->ship[0].getPosition() +
                    sf::Vector2f(GameScreen::playerEdges[i], GameScreen::playerEdges[i + 1]));
                if(Utility::isWithinPolygon(coords, shipEdge.x, shipEdge.y))
                {
                    playerHurt(30);
                    break;
                }
            }

            // collision between asteroid and projectiles
            if(gc->gameManager.hasTag<TAsteroid>(eid))
            {
                asteroidPos = pos;
                gc->gameManager.forMatchingSignature<EntitySignature>([this, &gc, &dt, &coords, &context]
                   (std::size_t eid,
                    Position& pos,
                    Velocity& vel,
                    Acceleration& acc,
                    Rotation& rotation,
                    AngularVelocity& vrotation,
                    Offset& offset,
                    Size& size)
                {
                    if(gc->gameManager.hasTag<TProjectile>(eid) && Utility::isWithinPolygon(coords, pos.x, pos.y))
                    {
                        this->asteroidHurt(1, context);
                        gc->gameManager.deleteEntity(eid);
                    }
                });
            }
        }
    });

    gc->gameManager.forMatchingSignature<EC::Meta::TypeList<Timer> >([this, &dt, &gc]
        (std::size_t eid,
        Timer& timer)
    {
        timer.time += dt.asSeconds();
        if((gc->gameManager.hasTag<TParticle>(eid) && timer.time >= GAME_ASTEROID_PARTICLE_LIFETIME) ||
            (gc->gameManager.hasTag<TProjectile>(eid) && timer.time >= GAME_FIRE_LIFETIME) ||
            (gc->gameManager.hasTag<TExplosion>(eid) && timer.time >= GAME_EXPLOSION_LIFETIME) ||
            (gc->gameManager.hasTag<TAsteroidProjectile>(eid) &&
                (
                (this->asteroidPhase == 1 && timer.time >= GAME_ASTEROID_PROJECTILE_LIFETIME) ||
                (this->asteroidPhase == 2 && timer.time >= GAME_ASTEROID_PROJECTILE_PHASE_2_LIFETIME) ||
                (this->asteroidPhase == 3 && timer.time >= GAME_ASTEROID_PROJECTILE_PHASE_3_LIFETIME) ||
                (this->asteroidPhase == 4 && timer.time >= GAME_ASTEROID_PROJECTILE_PHASE_4_LIFETIME) ||
                (this->asteroidPhase == 5 && timer.time >= GAME_ASTEROID_PROJECTILE_PHASE_5_LIFETIME)
                )
            ))
        {
            gc->gameManager.deleteEntity(eid);
        }
    });

    if(asteroidPhase <= 6)
    {
        auto eid = gc->gameManager.addEntity();
        gc->gameManager.addComponent<Position>(eid, 
            asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
            asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
        );
        gc->gameManager.addComponent<Velocity>(eid);
        gc->gameManager.addComponent<Acceleration>(eid,
            (rdist(*context.randomEngine) * 80.0f - 40.0f),
            -200.0f + (rdist(*context.randomEngine) * 80.0f - 40.0f)
        );
        gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
        gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 70.0f);
        gc->gameManager.addComponent<Offset>(eid, 8.0f, 8.0f);
        gc->gameManager.addComponent<Size>(eid, 16.0f, 16.0f);
        gc->gameManager.addComponent<Timer>(eid);
        gc->gameManager.addTag<TParticle>(eid);
    }

    ++garbageTimer;
    if(garbageTimer >= 300)
    {
        gc->gameManager.cleanup();
        garbageTimer = 0;
    }

    volumeButton.setPosition(view.getCenter().x - view.getSize().x / 2.0f, view.getCenter().y - view.getSize().y / 2.0f);

    // asteroid phase attacks
    if(asteroidPhase == 1)
    {
        asteroidFireTimer += dt.asSeconds();
        if(asteroidFireTimer >= GAME_ASTEROID_PHASE_1_FIRE_TIME)
        {
            asteroidFireTimer = 0;

            sf::Vector2f aProjectilePos(
                asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
                asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
            );

            sf::Vector2f dir = ship[0].getPosition() - aProjectilePos;
            dir /= std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir *= GAME_ASTEROID_PHASE_1_FIRE_SPEED;

            auto eid = gc->gameManager.addEntity();
            gc->gameManager.addComponent<Position>(eid,
                aProjectilePos.x,
                aProjectilePos.y
            );
            gc->gameManager.addComponent<Velocity>(eid,
                dir.x,
                dir.y
            );
            gc->gameManager.addComponent<Acceleration>(eid);
            gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
            gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 30.0f);
            gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
            gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
            gc->gameManager.addComponent<Timer>(eid);
            gc->gameManager.addTag<TAsteroidProjectile>(eid);
        }
    }
    else if(asteroidPhase == 2)
    {
        asteroidFireTimer += dt.asSeconds();
        if(asteroidFireTimer >= GAME_ASTEROID_PHASE_2_FIRE_TIME)
        {
            asteroidFireTimer = 0;

            for(int i = 0; i < 3; ++i)
            {
                sf::Vector2f aProjectilePos(
                    asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
                    asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
                );

                sf::Vector2f dir = ship[0].getPosition() - aProjectilePos;
                dir /= std::sqrt(dir.x * dir.x + dir.y * dir.y);
                dir *= GAME_ASTEROID_PHASE_2_FIRE_SPEED;

                sf::Transform transform;
                transform.rotate((i - 1) * 30);
                dir = transform * dir;

                auto eid = gc->gameManager.addEntity();
                gc->gameManager.addComponent<Position>(eid,
                    aProjectilePos.x,
                    aProjectilePos.y
                );
                gc->gameManager.addComponent<Velocity>(eid,
                    dir.x,
                    dir.y
                );
                gc->gameManager.addComponent<Acceleration>(eid);
                gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
                gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 30.0f);
                gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
                gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
                gc->gameManager.addComponent<Timer>(eid);
                gc->gameManager.addTag<TAsteroidProjectile>(eid);
            }
        }
    }
    else if(asteroidPhase == 3)
    {
        asteroidFireTimer += dt.asSeconds();
        if(asteroidFireTimer >= GAME_ASTEROID_PHASE_3_FIRE_TIME)
        {
            asteroidFireTimer = 0;

            sf::Vector2f aProjectilePos(
                asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
                asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
            );

            sf::Vector2f dir = ship[0].getPosition() - aProjectilePos;
            dir /= std::sqrt(dir.x * dir.x + dir.y * dir.y);
            sf::Vector2f vel = dir * GAME_ASTEROID_PHASE_3_FIRE_SPEED;
            sf::Vector2f accel = dir * GAME_ASTEROID_PHASE_3_FIRE_ACCEL;

            auto eid = gc->gameManager.addEntity();
            gc->gameManager.addComponent<Position>(eid,
                aProjectilePos.x,
                aProjectilePos.y
            );
            gc->gameManager.addComponent<Velocity>(eid,
                vel.x,
                vel.y
            );
            gc->gameManager.addComponent<Acceleration>(eid,
                accel.x,
                accel.y
            );
            gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
            gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 30.0f);
            gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
            gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
            gc->gameManager.addComponent<Timer>(eid);
            gc->gameManager.addTag<TAsteroidProjectile>(eid);


            vel = dir * GAME_ASTEROID_PHASE_3_FIRE_ALT_SPEED;
            eid = gc->gameManager.addEntity();
            gc->gameManager.addComponent<Position>(eid,
                asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
                asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
            );
            gc->gameManager.addComponent<Velocity>(eid,
                vel.x,
                vel.y
            );
            gc->gameManager.addComponent<Acceleration>(eid);
            gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
            gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 30.0f);
            gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
            gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
            gc->gameManager.addComponent<Timer>(eid);
            gc->gameManager.addTag<TAsteroidProjectile>(eid);
        }
    }
    else if(asteroidPhase == 4)
    {
        asteroidFireTimer += dt.asSeconds();
        if(asteroidFireTimer >= GAME_ASTEROID_PHASE_4_FIRE_TIME)
        {
            asteroidFireTimer = 0;

            sf::Vector2f dir(
                asteroidPos.x,
                asteroidPos.y
            );

            dir = ship[0].getPosition() - dir;
            dir /= std::sqrt(dir.x * dir.x + dir.y * dir.y);

            sf::Transform transform;

            for(unsigned int i = 0; i < 8; ++i)
            {
                transform.rotate(45);

                sf::Vector2f vel = transform * dir * GAME_ASTEROID_PHASE_4_FIRE_SPEED;

                auto eid = gc->gameManager.addEntity();
                gc->gameManager.addComponent<Position>(eid,
                    asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
                    asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
                );
                gc->gameManager.addComponent<Velocity>(eid,
                    vel.x,
                    vel.y
                );
                gc->gameManager.addComponent<Acceleration>(eid);
                gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
                gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 30.0f);
                gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
                gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
                gc->gameManager.addComponent<Timer>(eid);
                gc->gameManager.addTag<TAsteroidProjectile>(eid);
            }
        }
    }
    else if(asteroidPhase == 5)
    {
        asteroidFireTimer += dt.asSeconds();
        if(asteroidFireTimer >= GAME_ASTEROID_PHASE_5_FIRE_TIME)
        {
            asteroidFireTimer = 0;

            sf::Vector2f dir(
                asteroidPos.x,
                asteroidPos.y
            );

            dir = ship[0].getPosition() - dir;
            dir /= std::sqrt(dir.x * dir.x + dir.y * dir.y);

            sf::Transform transform;
            transform.rotate(45);

            for(unsigned int i = 0; i < 4; ++i)
            {
                transform.rotate(90);

                sf::Vector2f vel = transform * dir * GAME_ASTEROID_PHASE_5_FIRE_SPEED;

                auto eid = gc->gameManager.addEntity();
                gc->gameManager.addComponent<Position>(eid,
                    asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
                    asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
                );
                gc->gameManager.addComponent<Velocity>(eid,
                    vel.x,
                    vel.y
                );
                gc->gameManager.addComponent<Acceleration>(eid);
                gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
                gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 30.0f);
                gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
                gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
                gc->gameManager.addComponent<Timer>(eid);
                gc->gameManager.addTag<TAsteroidProjectile>(eid);
            }

            sf::Vector2f vel = dir * GAME_ASTEROID_PHASE_5_FIRE_ALT_SPEED;

            auto eid = gc->gameManager.addEntity();
            gc->gameManager.addComponent<Position>(eid,
                asteroidPos.x + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
                asteroidPos.y + (rdist(*context.randomEngine) * 128.0f) - 64.0f
            );
            gc->gameManager.addComponent<Velocity>(eid,
                vel.x,
                vel.y
            );
            gc->gameManager.addComponent<Acceleration>(eid);
            gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
            gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 30.0f);
            gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
            gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
            gc->gameManager.addComponent<Timer>(eid);
            gc->gameManager.addTag<TAsteroidProjectile>(eid);
        }
    }

    return false;
}

bool GameScreen::handleEvent(const sf::Event& event, Context context)
{
    if(event.type == sf::Event::MouseMoved)
    {
//        mousePos = context.window->mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        mousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
    }
    else if(event.type == sf::Event::KeyPressed)
    {
        if(event.key.code == sf::Keyboard::W)
        {
            flags |= 0x10;
        }
        else if(event.key.code == sf::Keyboard::S)
        {
            flags |= 0x20;
        }
        else if(event.key.code == sf::Keyboard::A)
        {
            flags |= 0x40;
        }
        else if(event.key.code == sf::Keyboard::D)
        {
            flags |= 0x80;
        }
        else if(event.key.code == sf::Keyboard::Space)
        {
            flags |= 0x8;
        }
    }
    else if(event.type == sf::Event::KeyReleased)
    {
        if(event.key.code == sf::Keyboard::W)
        {
            flags &= 0xFFFFFFFFFFFFFFEF;
        }
        else if(event.key.code == sf::Keyboard::S)
        {
            flags &= 0xFFFFFFFFFFFFFFDF;
        }
        else if(event.key.code == sf::Keyboard::A)
        {
            flags &= 0xFFFFFFFFFFFFFFBF;
        }
        else if(event.key.code == sf::Keyboard::D)
        {
            flags &= 0xFFFFFFFFFFFFFF7F;
        }
        else if(event.key.code == sf::Keyboard::Space)
        {
            flags &= 0xFFFFFFFFFFFFFFF7;
        }
    }
    else if(event.type == sf::Event::MouseButtonPressed)
    {
        if(event.mouseButton.x >= 0 &&
            event.mouseButton.x <= 64 &&
            event.mouseButton.y >= 0 &&
            event.mouseButton.y <= 64)
        {
            flags = ((~flags) & 0x100) | (flags & 0xFFFFFFFFFFFFFEFF);
            if((flags & 0x100) != 0)
            {
                bgMusic.pause();
            }
            else
            {
                bgMusic.play();
            }
        }
        else
        {
            flags |= 0x8;
        }
    }
    else if(event.type == sf::Event::MouseButtonReleased)
    {
        flags &= 0xFFFFFFFFFFFFFFF7;
    }

    return false;
}

void GameScreen::animateShipThruster(sf::Time dt)
{
    for(unsigned int i = 0; i < 2; ++i)
    {
        shipTimer[i] += dt.asSeconds();
    }

    if(shipTimer[0] >= GAME_THRUSTER_FLICKER_RATE)
    {
        shipTimer[0] -= GAME_THRUSTER_FLICKER_RATE;
        ship[1].setTextureRect(sf::IntRect((flags & 0x2) == 0 ? 128.0f : 64.0f, 0.0f, 64.0f, 160.0f));
        flags = ((~flags) & 0x2) | (flags & 0xFFFFFFFFFFFFFFFD);
    }

    if(shipTimer[1] >= GAME_THRUSTER_COLOR_FADE_RATE)
    {
        shipTimer[1] -= GAME_THRUSTER_COLOR_FADE_RATE;
        flags = ((~flags) & 0x4) | (flags & 0xFFFFFFFFFFFFFFFB);
#ifndef NDEBUG
//        std::cout << "flags " << std::hex << flags << std::dec << std::endl;
#endif
    }
    if((flags & 0x4) == 0)
    {
        if(playerHP > 0)
        {
            ship[1].setColor(sf::Color(
                127,
                (unsigned char)(shipTimer[1] / GAME_THRUSTER_COLOR_FADE_RATE * 255.0f),
                (unsigned char)((1.0f - shipTimer[1] / GAME_THRUSTER_COLOR_FADE_RATE) * 255.0f)
            ));
        }
    }
    else
    {
        if(playerHP > 0)
        {
            ship[1].setColor(sf::Color(
                127,
                (unsigned char)((1.0f - shipTimer[1] / GAME_THRUSTER_COLOR_FADE_RATE) * 200.0f + 55.0f),
                (unsigned char)(shipTimer[1] / GAME_THRUSTER_COLOR_FADE_RATE * 200.0f + 55.0f)
            ));
        }
    }
}

void GameScreen::playerInput(sf::Time dt, Context context)
{
    if(playerHP <= 0)
    {
        return;
    }
    if((flags & 0x10) != 0 && (flags & 0x20) == 0)
    {
        if(((flags & 0x40) == 0 && (flags & 0x80) == 0) ||
            ((flags & 0x40) != 0 && (flags & 0x80) != 0))
        {
            shipPos.y -= GAME_PLAYER_SPEED * dt.asSeconds();
        }
        else if((flags & 0x40) != 0 && (flags & 0x80) == 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.y -= amount;
            shipPos.x -= amount;
        }
        else if((flags & 0x40) == 0 && (flags & 0x80) != 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.y -= amount;
            shipPos.x += amount;
        }
    }
    else if((flags & 0x20) != 0 && (flags & 0x10) == 0)
    {
        if(((flags & 0x40) == 0 && (flags & 0x80) == 0) ||
            ((flags & 0x40) != 0 && (flags & 0x80) != 0))
        {
            shipPos.y += GAME_PLAYER_SPEED * dt.asSeconds();
        }
        else if((flags & 0x40) != 0 && (flags & 0x80) == 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.y += amount;
            shipPos.x -= amount;
        }
        else if((flags & 0x40) == 0 && (flags & 0x80) != 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.y += amount;
            shipPos.x += amount;
        }
    }
    else if((flags & 0x40) != 0 && (flags & 0x80) == 0)
    {
        if(((flags & 0x10) == 0 && (flags & 0x20) == 0) ||
            ((flags & 0x10) != 0 && (flags & 0x20) != 0))
        {
            shipPos.x -= GAME_PLAYER_SPEED * dt.asSeconds();
        }
        else if((flags & 0x10) != 0 && (flags & 0x20) == 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.x -= amount;
            shipPos.y -= amount;
        }
        else if((flags & 0x10) == 0 && (flags & 0x20) != 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.x -= amount;
            shipPos.y += amount;
        }
    }
    else if((flags & 0x40) == 0 && (flags & 0x80) != 0)
    {
        if(((flags & 0x10) == 0 && (flags & 0x20) == 0) ||
            ((flags & 0x10) != 0 && (flags & 0x20) != 0))
        {
            shipPos.x += GAME_PLAYER_SPEED * dt.asSeconds();
        }
        else if((flags & 0x10) != 0 && (flags & 0x20) == 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.x += amount;
            shipPos.y -= amount;
        }
        else if((flags & 0x10) == 0 && (flags & 0x20) != 0)
        {
            float amount = GAME_PLAYER_SPEED * dt.asSeconds() * std::sqrt(2.0f) / 2.0f;
            shipPos.x += amount;
            shipPos.y += amount;
        }
    }

    sf::Vector2f windowSize = view.getSize();

    if(shipPos.x - GAME_SHIP_LIMIT_RADIUS < 0)
    {
        shipPos.x = GAME_SHIP_LIMIT_RADIUS;
    }
    else if(shipPos.x + GAME_SHIP_LIMIT_RADIUS > windowSize.x)
    {
        shipPos.x = windowSize.x - GAME_SHIP_LIMIT_RADIUS;
    }

    if(shipPos.y - GAME_SHIP_LIMIT_RADIUS < 0)
    {
        shipPos.y = GAME_SHIP_LIMIT_RADIUS;
    }
    else if(shipPos.y + GAME_SHIP_LIMIT_RADIUS > windowSize.y)
    {
        shipPos.y = windowSize.y - GAME_SHIP_LIMIT_RADIUS;
    }

    GameContext* gc = static_cast<GameContext*>(context.extraContext);

    if((flags & 0x8) != 0 && fireTimer == 0.0f)
    {
        fireTimer = GAME_FIRE_TIME;

        auto eid = gc->gameManager.addEntity();
        gc->gameManager.addComponent<Position>(eid,
            ship[0].getPosition().x,
            ship[0].getPosition().y
        );
        float rot = ship[0].getRotation() - 90;
        float rotRadians = rot * std::acos(-1) / 180;
        gc->gameManager.addComponent<Velocity>(eid,
            GAME_FIRE_VELOCITY * std::cos(rotRadians),
            GAME_FIRE_VELOCITY * std::sin(rotRadians)
        );
        gc->gameManager.addComponent<Acceleration>(eid);
        gc->gameManager.addComponent<Rotation>(eid, rot);
        gc->gameManager.addComponent<AngularVelocity>(eid);
        gc->gameManager.addComponent<Offset>(eid, 4.0f, 4.0f);
        gc->gameManager.addComponent<Size>(eid, 8.0f, 8.0f);
        gc->gameManager.addComponent<Timer>(eid);
        gc->gameManager.addTag<TProjectile>(eid);
    }
}

void GameScreen::playerHurt(int damage)
{
    if(playerInvisTime != 0.0f)
    {
        return;
    }

    playerHP -= damage;
    playerInvisTime = GAME_PLAYER_INVIS_TIME;

    if(playerHP <= 0)
    {
        for(unsigned int i = 0; i < 2; ++i)
        {
            ship[i].setColor(sf::Color(0, 0, 0, 0));
        }
    }
    else
    {
        ship[0].setColor(sf::Color::Red);
    }
}

void GameScreen::updateHealthBar(Context context)
{
    float percentage = (float)(playerHP > 0 ? playerHP : 0) / GAME_MAX_PLAYER_HP;

    float offsety = 160.0f - 160.0f * percentage;

    healthBar[0].setTextureRect(sf::IntRect(0, offsety, 64, 160.0f * percentage));

    sf::Vector2f windowPos = context.window->getView().getCenter() - context.window->getView().getSize() / 2.0f;

    healthBar[0].setPosition(windowPos.x, windowPos.y + offsety + 540.0f - 160.0f);
    healthBar[1].setPosition(windowPos.x, windowPos.y + 540.0f - 160.0f);
}

void GameScreen::asteroidHurt(int damage, Context context)
{
    asteroidHP -= damage;
    if(asteroidHP <= 0)
    {
        GameContext* gc = static_cast<GameContext*>(context.extraContext);
        switch(asteroidPhase)
        {
        case 0:
            asteroidPhase = 1;
            asteroidHP = GAME_ASTEROID_PHASE_1_HP;
            asteroidExplosionsCount = GAME_ASTEROID_PHASE_0_EXPLOSIONS;
            flags |= 0x200;
            break;
        case 1:
            asteroidPhase = 2;
            asteroidHP = GAME_ASTEROID_PHASE_2_HP;
            asteroidExplosionsCount = GAME_ASTEROID_PHASE_0_EXPLOSIONS;
            flags |= 0x400;
            break;
        case 2:
            asteroidPhase = 3;
            asteroidHP = GAME_ASTEROID_PHASE_3_HP;
            asteroidExplosionsCount = GAME_ASTEROID_PHASE_0_EXPLOSIONS;
            flags |= 0x200;
            break;
        case 3:
        {
            asteroidPhase = 4;
            asteroidHP = GAME_ASTEROID_PHASE_4_HP;
            asteroidExplosionsCount = GAME_ASTEROID_PHASE_0_EXPLOSIONS;
            flags |= 0x200;
            Velocity& aVel = gc->gameManager.getEntityData<Velocity>(asteroidID);
            aVel.y = GAME_CENTER_ASTEROID_SPEED;
        }
            break;
        case 4:
            asteroidPhase = 5;
            asteroidHP = GAME_ASTEROID_PHASE_5_HP;
            asteroidExplosionsCount = GAME_ASTEROID_PHASE_0_EXPLOSIONS;
            flags |= 0x200;
            break;
        case 5:
            asteroidPhase = 6;
            timer = 0;
            asteroidExplosionsCount = 0xFFFFFFFF;
        default:
            break;
        }
    }
}

void GameScreen::checkAsteroidExplosions(sf::Time dt, Context context)
{
    asteroidExplosionsTimer -= dt.asSeconds();
    if(asteroidExplosionsTimer <= 0)
    {
        asteroidExplosionsTimer = GAME_ASTEROID_EXPLOSIONS_INTERVAL;
        if(asteroidExplosionsCount > 0 && asteroidPhase <= 6)
        {
            --asteroidExplosionsCount;

            GameContext* gc = static_cast<GameContext*>(context.extraContext);

            Position& asteroidPosition = gc->gameManager.getEntityData<Position>(asteroidID);

            auto eid = gc->gameManager.addEntity();
            gc->gameManager.addComponent<Position>(eid,
                asteroidPosition.x + rdist(*context.randomEngine) * 128.0f - 64.0f,
                asteroidPosition.y + rdist(*context.randomEngine) * 128.0f - 64.0f
            );
            gc->gameManager.addComponent<Velocity>(eid);
            gc->gameManager.addComponent<Acceleration>(eid);
            gc->gameManager.addComponent<Rotation>(eid);
            gc->gameManager.addComponent<AngularVelocity>(eid);
            gc->gameManager.addComponent<Offset>(eid, 16.0f, 16.0f);
            gc->gameManager.addComponent<Size>(eid, 32.0f, 32.0f);
            gc->gameManager.addComponent<Timer>(eid);
            gc->gameManager.addTag<TExplosion>(eid);
        }
    }
}

