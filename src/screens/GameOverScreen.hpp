
#ifndef GAMEOVER_SCREEN_HPP
#define GAMEOVER_SCREEN_HPP

#include <engine/state.hpp>

#include <SFML/Graphics.hpp>

class GameOverScreen : public State
{
public:
    GameOverScreen(StateStack& stack, Context context);
    virtual ~GameOverScreen() override;

    virtual void draw(Context context) override;
    virtual bool update(sf::Time dt, Context context) override;
    virtual bool handleEvent(const sf::Event& event, Context context) override;

private:
    sf::Sprite retryButton;

    void retry();

};

#endif

