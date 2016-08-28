
#include <engine/game.hpp>
#include "GameContext.hpp"

#include "screens/GameScreen.hpp"
#include "screens/GameOverScreen.hpp"

int main(int argc, char** argv)
{
    GameContext gameContext;

    Game game(&gameContext);

    game.registerState<GameScreen>("GameScreen");
    game.registerState<GameOverScreen>("GameOver");
    game.setStartingState("GameScreen");

    game.run();

    return 0;
}

