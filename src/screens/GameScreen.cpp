
#include "GameScreen.hpp"

#include "components/Components.hpp"
#include "GameContext.hpp"

#include <EC/Meta/Meta.hpp>

GameScreen::GameScreen(StateStack& stack, Context context) :
State(stack, context),
flags(0),
rdist(),
garbageTimer(0),
drawFrameTimer(0)
{
    GameContext* gc = static_cast<GameContext*>(context.extraContext);

    auto eid = gc->gameManager.addEntity();
    gc->gameManager.addComponent<Position>(eid, 480.0f, 270.0f);
    gc->gameManager.addComponent<Velocity>(eid);
    gc->gameManager.addComponent<Acceleration>(eid);
    gc->gameManager.addComponent<Rotation>(eid);
    gc->gameManager.addComponent<AngularVelocity>(eid, 70.0f);
    gc->gameManager.addComponent<Offset>(eid, 64.0f, 64.0f);
    gc->gameManager.addComponent<Size>(eid, 128.0f, 128.0f);
    gc->gameManager.addTag<TAsteroid>(eid);
    asteroidID = eid;

    context.window->setFramerateLimit(60);
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
            float r = rdist(*context.randomEngine);
            this->drawRect.setFillColor(sf::Color(
                (unsigned char)(r * 100.0f + 100.0f),
                (unsigned char)(r * 35.0f + 35.0f),
                (unsigned char)(r * 35.0f + 35.0f)
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

    if(++drawFrameTimer >= 60)
    {
        drawFrameTimer -= 60;
    }
}

bool GameScreen::update(sf::Time dt, Context context)
{
    GameContext* gc = static_cast<GameContext*>(context.extraContext);

    sf::Vector2f windowSize = context.window->getView().getSize();

    gc->gameManager.forMatchingSignature<EntitySignature>([&gc, &dt, &windowSize]
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

        if(gc->gameManager.hasTag<TParticle>(eid))
        {
            if(pos.x > windowSize.x || pos.y + size.height < 0)
            {
                gc->gameManager.deleteEntity(eid);
            }
        }
    });

    auto eid = gc->gameManager.addEntity();
    gc->gameManager.addComponent<Position>(eid, 
        480.0f + (rdist(*context.randomEngine) * 128.0f) - 64.0f,
        270.0f + (rdist(*context.randomEngine) * 128.0f) - 64.0f
    );
    gc->gameManager.addComponent<Velocity>(eid);
    gc->gameManager.addComponent<Acceleration>(eid,
        300.0f + (rdist(*context.randomEngine) * 80.0f - 40.0f),
        -200.0f + (rdist(*context.randomEngine) * 80.0f - 40.0f)
    );
    gc->gameManager.addComponent<Rotation>(eid, rdist(*context.randomEngine) * 360.0f);
    gc->gameManager.addComponent<AngularVelocity>(eid, rdist(*context.randomEngine) * 70.0f);
    gc->gameManager.addComponent<Offset>(eid, 8.0f, 8.0f);
    gc->gameManager.addComponent<Size>(eid, 16.0f, 16.0f);
    gc->gameManager.addTag<TParticle>(eid);

    ++garbageTimer;
    if(garbageTimer >= 300)
    {
        gc->gameManager.cleanup();
        garbageTimer = 0;
    }

    return false;
}

bool GameScreen::handleEvent(const sf::Event& event, Context context)
{
    return false;
}

