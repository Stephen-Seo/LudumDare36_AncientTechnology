
#include "GameScreen.hpp"

#include <cmath>

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
playerRegenTimer(0)
{
    context.resourceManager->registerTexture(*this, "res/Planet.png");
    context.resourceManager->registerTexture(*this, "res/ship.png");
    context.resourceManager->registerTexture(*this, "res/volumeButton.png");
    context.resourceManager->registerTexture(*this, "res/healthBar.png");
    context.resourceManager->loadResources();

    sf::Texture& planetTexture = context.resourceManager->getTexture("res/Planet.png");
    sf::Texture& shipTexture = context.resourceManager->getTexture("res/ship.png");
    sf::Texture& healthBarTexture = context.resourceManager->getTexture("res/healthBar.png");

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

    gc->gameManager.forMatchingSignature<EntitySignature>([this, &context, &gc]
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
        else
        {
            this->drawRect.setFillColor(sf::Color::White);
        }

        this->drawRect.setSize(sf::Vector2f(size.width, size.height));
        this->drawRect.setPosition(pos.x, pos.y);
        this->drawRect.setOrigin(offset.x, offset.y);
        this->drawRect.setRotation(rotation.rotation);

        context.window->draw(this->drawRect);
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

    if(drawFrameTimer % 10 == 0)
    {
        float r = rdist(*context.randomEngine);
        asteroidColor = sf::Color(
            (unsigned char)(r * 100.0f + 100.0f),
            (unsigned char)(r * 35.0f + 35.0f),
            (unsigned char)(r * 35.0f + 35.0f)
        );
    }
    drawRect.setFillColor(asteroidColor);

    context.window->draw(drawRect);

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

    if(++drawFrameTimer >= 60)
    {
        drawFrameTimer -= 60;
    }
}

bool GameScreen::update(sf::Time dt, Context context)
{
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

    updateHealthBar(context);

    playerInput(dt);

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

    GameContext* gc = static_cast<GameContext*>(context.extraContext);

    gc->gameManager.forMatchingSignature<EntitySignature>([this, &gc, &dt, &asteroidPos]
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

        if(gc->gameManager.hasTag<TAsteroid>(eid))
        {
            asteroidPos = pos;

            // collision detection between player and asteroid
            // x = x * cos + y * sin
            // y = x * -sin + y * cos
            // [  cos  sin ]
            // [ -sin  cos ]
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
#ifndef NDEBUG
//            bool noCollide = true;
#endif
            for(unsigned int i = 0; i < 16; i += 2)
            {
                shipEdge = transform * (this->ship[0].getPosition() +
                    sf::Vector2f(GameScreen::playerEdges[i], GameScreen::playerEdges[i + 1]));
                if(Utility::isWithinPolygon(coords, shipEdge.x, shipEdge.y))
                {
//                    this->ship[0].setColor(sf::Color::Red);
                    playerHurt(30);
#ifndef NDEBUG
//                    std::cout << "colliding" << std::endl;
//                    noCollide = false;
#endif
                    break;
                }
//                    this->ship[0].setColor(sf::Color::White);
            }
#ifndef NDEBUG
/*
            if(noCollide)
            {
                std::cout << "Not colliding" << std::endl;
            }
*/
#endif
        }
    });

    gc->gameManager.forMatchingSignature<EC::Meta::TypeList<Timer> >([&dt, &gc]
        (std::size_t eid,
        Timer& timer)
    {
        timer.time += dt.asSeconds();
        if(gc->gameManager.hasTag<TParticle>(eid) && timer.time >= GAME_ASTEROID_PARTICLE_LIFETIME)
        {
            gc->gameManager.deleteEntity(eid);
        }
    });

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

    ++garbageTimer;
    if(garbageTimer >= 300)
    {
        gc->gameManager.cleanup();
        garbageTimer = 0;
    }

    volumeButton.setPosition(view.getCenter().x - view.getSize().x / 2.0f, view.getCenter().y - view.getSize().y / 2.0f);

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

void GameScreen::playerInput(sf::Time dt)
{
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

