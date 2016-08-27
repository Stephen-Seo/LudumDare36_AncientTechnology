
#ifndef GAME_CONTEXT_HPP
#define GAME_CONTEXT_HPP

#include <EC/EC.hpp>
#include <components/Components.hpp>

struct GameContext
{
    using GameManager = EC::Manager<GameComponentsList, GameTagsList>;
    GameManager gameManager;
};

#endif

