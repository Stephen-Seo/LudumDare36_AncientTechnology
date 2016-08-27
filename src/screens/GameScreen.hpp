
#ifndef GAME_SCREEN_HPP
#define GAME_SCREEN_HPP

#define GAME_ASTEROID_PARTICLE_LIFETIME 3.0f
#define GAME_THRUSTER_FLICKER_RATE 0.04f
#define GAME_THRUSTER_COLOR_FADE_RATE 1.5f
#define GAME_PLAYER_SPEED 200.0f

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
    /*
        0001 - panned to asteroid
        0010 - shipThruster flicker
        0100 - shipThruster color reverse
        1000 - space (fire)

        0001 0000 - up
        0010 0000 - down
        0100 0000 - left
        1000 0000 - right
    */
    uint64_t flags;
    std::uniform_real_distribution<> rdist;
    unsigned int garbageTimer;
    std::size_t asteroidID;
    unsigned int drawFrameTimer;
    sf::Color asteroidColor;
    sf::RectangleShape planetRect;
    sf::View view;
    float timer;
    sf::Sprite ship[2];
    float shipTimer[2];
    sf::Vector2f shipPos;

    void animateShipThruster(sf::Time dt);
    void playerInput(sf::Time dt);

};

#endif

