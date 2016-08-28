
#include "GameOverScreen.hpp"

#include <engine/resourceManager.hpp>

GameOverScreen::GameOverScreen(StateStack& stack, Context context) :
State(stack, context)
{
    context.resourceManager->registerTexture(*this, "res/retryButton.png");

    context.resourceManager->loadResources();

    sf::Texture& retryTexture = context.resourceManager->getTexture("res/retryButton.png");

    retryButton.setTexture(retryTexture, true);

    retryButton.setPosition(960.0f / 2.0f, 540.0f / 2.0f);
    auto bounds = retryButton.getLocalBounds();
    retryButton.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
}

GameOverScreen::~GameOverScreen()
{
}

void GameOverScreen::draw(Context context)
{
    context.window->draw(retryButton);
}

bool GameOverScreen::update(sf::Time dt, Context context)
{
}

bool GameOverScreen::handleEvent(const sf::Event& event, Context context)
{
    auto bounds = retryButton.getLocalBounds();
    if(event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.x >= 960.0f / 2.0f - bounds.width / 2.0f &&
        event.mouseButton.x <= 960.0f / 2.0f + bounds.width / 2.0f &&
        event.mouseButton.y >= 540.0f / 2.0f - bounds.height / 2.0f &&
        event.mouseButton.y <= 540.0f / 2.0f + bounds.height / 2.0f)
    {
        retry();
    }
    else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
    {
        retry();
    }
}

void GameOverScreen::retry()
{
    requestStackClear();
    requestStackPush("GameScreen");
}

