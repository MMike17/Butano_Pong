#include "game_state.h"

int get_gamestate_size()
{
	return sizeof(GameStateStrings) / sizeof(*GameStateStrings);
}