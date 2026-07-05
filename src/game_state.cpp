#include "game_state.h"
#include "bn_string_view.h"
#include "bn_array.h"

const int GAMESTATE_SIZE = 3;
const bn::array<bn::string_view, 3> GameStateStrings[]{"Intro", "Game", "Result"};