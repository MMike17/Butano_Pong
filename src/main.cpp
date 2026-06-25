#include "bn_core.h"
#include "bn_keypad.h"
// #include "bn_common_fixed"

// sprite imports
#include "bn_sprite_items_paddle.h"

enum gameState : int
{
	Intro,
	Game,
	Result
};

gameState state;
bool inIntro;

int main()
{
	Init();

	while (true)
	{
		Update();
		bn::core::update();
	}
}

void Init()
{
	state = gameState::Intro;
	inIntro = false;

	bn::core::init();
}

void Update()
{
	// how do I setup text screens for intro and outro ?

	switch (state)
	{
	case gameState::Intro:
		// display text here
		if (bn::keypad::held(bn::keypad::key_type::A))
			state = gameState::Game;
		break;

	case gameState::Game:
		// game loop here
		break;

	case gameState::Result:
		// display results and reset on press A
		break;
	}
}