
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
#define GAME_FIRE_TIME 0.35f
#define GAME_FIRE_VELOCITY 800.0f
#define GAME_FIRE_LIFETIME 4.0f
#define GAME_ASTEROID_PHASE_0_HP 20
#define GAME_ASTEROID_PHASE_1_HP 40
#define GAME_ASTEROID_PHASE_2_HP 50
#define GAME_ASTEROID_PHASE_3_HP 50
#define GAME_ASTEROID_PHASE_4_HP 50
#define GAME_ASTEROID_PHASE_5_HP 50
#define GAME_EXPLOSION_LIFETIME 1.5f
#define GAME_ASTEROID_PHASE_0_EXPLOSIONS 7
#define GAME_ASTEROID_EXPLOSIONS_INTERVAL 0.2f
#define GAME_SCREEN_FLASH_FAST_TIME 0.5f
#define GAME_SCREEN_FLASH_SLOW_TIME 1.2f
#define GAME_ASTEROID_PHASE_1_FIRE_TIME 0.5f
#define GAME_ASTEROID_PHASE_2_FIRE_TIME 0.9f
#define GAME_ASTEROID_PHASE_3_FIRE_TIME 0.7f
#define GAME_ASTEROID_PHASE_4_FIRE_TIME 0.6f
#define GAME_ASTEROID_PHASE_5_FIRE_TIME 0.4f
#define GAME_ASTEROID_PHASE_1_FIRE_SPEED 350.0f
#define GAME_ASTEROID_PHASE_2_FIRE_SPEED 100.0f
#define GAME_ASTEROID_PHASE_3_FIRE_SPEED 200.0f
#define GAME_ASTEROID_PHASE_3_FIRE_ALT_SPEED 300.0f
#define GAME_ASTEROID_PHASE_3_FIRE_ACCEL 800.0f
#define GAME_ASTEROID_PHASE_4_FIRE_SPEED 300.0f
#define GAME_ASTEROID_PHASE_5_FIRE_SPEED 200.0f
#define GAME_ASTEROID_PHASE_5_FIRE_ALT_SPEED 400.0f
#define GAME_ASTEROID_PROJECTILE_LIFETIME 4.0f
#define GAME_ASTEROID_PROJECTILE_PHASE_2_LIFETIME 8.0f
#define GAME_ASTEROID_PROJECTILE_PHASE_3_LIFETIME 5.0f
#define GAME_ASTEROID_PROJECTILE_PHASE_4_LIFETIME 5.0f
#define GAME_ASTEROID_PROJECTILE_PHASE_5_LIFETIME 5.0f
#define GAME_CENTER_ASTEROID_SPEED 50.0f
#define GAME_ASTEROID_DEATH_TIME 7.0f

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
        1000 - fire

        0001 0000 - up
        0010 0000 - down
        0100 0000 - left
        1000 0000 - right

        0001 0000 0000 - mute music
        0010 0000 0000 - screen flash fast
        0100 0000 0000 - screen flash slow
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
    float fireTimer;
    int asteroidHP;
    unsigned int asteroidPhase;
    sf::Texture* asteroidSymbolTexture;
    sf::CircleShape drawCircle;
    float asteroidExplosionsTimer;
    unsigned int asteroidExplosionsCount;
    float screenFlashTimer;
    float asteroidFireTimer;
    float asteroidRotationAngle;

    void animateShipThruster(sf::Time dt);
    void playerInput(sf::Time dt, Context context);
    void playerHurt(int damage);
    void updateHealthBar(Context context);
    void asteroidHurt(int damage, Context context);
    void checkAsteroidExplosions(sf::Time dt, Context context);

};

#endif

