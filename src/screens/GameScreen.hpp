
#ifndef GAME_SCREEN_HPP
#define GAME_SCREEN_HPP

#include <engine/state.hpp>

#include <cstdint>
#include <random>

#include <SFML/Graphics.hpp>

class GameScreen : public State
{
public:
    GameScreen(StateStack& stack, Context context);
    virtual ~GameScreen() override;

    virtual void draw(Context context) override;
    virtual bool update(sf::Time dt, Context context) override;
    virtual bool handleEvent(const sf::Event& event, Context context) override;

private:
    sf::RectangleShape drawRect;
    uint64_t flags;
    std::uniform_real_distribution<> rdist;
    unsigned int garbageTimer;
    std::size_t asteroidID;
    unsigned int drawFrameTimer;
    sf::Color asteroidColor;

};

#endif

