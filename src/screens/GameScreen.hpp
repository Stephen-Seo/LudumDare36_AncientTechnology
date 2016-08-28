
#ifndef GAME_SCREEN_HPP
#define GAME_SCREEN_HPP

#define GAME_ASTEROID_PARTICLE_LIFETIME 3.0f
#define GAME_THRUSTER_FLICKER_RATE 0.04f
#define GAME_THRUSTER_COLOR_FADE_RATE 1.5f
#define GAME_PLAYER_SPEED 200.0f
#define GAME_SHIP_LIMIT_RADIUS 16.0f
#define GAME_MAX_PLAYER_HP 100
#define GAME_PLAYER_INVIS_TIME 0.5f
#define GAME_PLAYER_REGEN_TIME 1.0f

#include <engine/state.hpp>

#include <cstdint>
#include <random>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class GameScreen : public State
{
public:
    GameScreen(StateStack& stack, Context context);
    virtual ~GameScreen() override;

    static const float playerEdges[16];

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

        0001 0000 0000 - mute music
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
    sf::Music bgMusic;
    sf::Vector2i mousePos;
    sf::Sprite volumeButton;
    int playerHP;
    float playerInvisTime;
    sf::Sprite healthBar[2];
    float playerRegenTimer;

    void animateShipThruster(sf::Time dt);
    void playerInput(sf::Time dt);
    void playerHurt(int damage);
    void updateHealthBar(Context context);

};

#endif

