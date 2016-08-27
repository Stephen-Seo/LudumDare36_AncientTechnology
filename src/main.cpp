
#include <engine/game.hpp>
#include "GameContext.hpp"

#include "screens/GameScreen.hpp"

int main(int argc, char** argv)
{
    GameContext gameContext;

    Game game(&gameContext);

    game.registerState<GameScreen>("GameScreen");
    game.setStartingState("GameScreen");

    game.run();

    return 0;
}

