#ifndef GAMESTATE_H_
#define GAMESTATE_H_

const enum GameState : int {
	Intro = 0,
	Game = 1,
	Pause = 2,
	Result = 3
};

const char *GameStateStrings[] = {"Intro", "Game", "Pause", "Result"};

#endif