// butano imports
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_text_generator.h"
#include "bn_log.h"

// custom imports
#include "main.h"
#include "canvas.h"

// sprite imports
#include "bn_sprite_items_common_fixed_8x8_font.h"

GameState state;
bn::vector<bn::sprite_ptr, 32> text_buffer;

void init()
{
	state = GameState::Intro;
	bn::core::init();
}

int main()
{
	init();

	// sets background color
	// bn::bg_palettes::set_transparent_color(bn::color(16, 16, 16));

	while (true)
	{
		state_update();
		bn::core::update();
	}
}

void state_update()
{
	switch (state)
	{
	case GameState::Intro:
		intro_display();
		intro_interraction();
		break;

	case GameState::Game:
		// TODO : game loop here
		break;

	case GameState::Result:
		// TODO : display results and reset on press A
		break;

	default:
		BN_LOG("Game state is broken");
		break;
	}
}

void switch_to_state(GameState newState)
{
	switch (state)
	{
	case GameState::Intro:
		text_buffer = bn::vector<bn::sprite_ptr, 32>();
		break;

	default: // this should never happen
		break;
	}

	state = newState;
	BN_LOG("Switched to state : ", GameStateStrings[newState]);
}

void intro_display()
{
	const bn::sprite_font font(bn::sprite_items::common_fixed_8x8_font);
	bn::sprite_text_generator text_generator(font);
	text_generator.set_center_alignment();
	text_buffer = bn::vector<bn::sprite_ptr, 32>();

	text_generator.generate(
		get_canvas_point(0.5f, true),
		get_canvas_point(0.7f, false),
		"Pong",
		text_buffer);
	text_generator.generate(
		get_canvas_point(0.5f, true),
		get_canvas_point(0.3f, false),
		"Press [start]",
		text_buffer);
	text_generator.generate(
		get_canvas_point(0.5f, true),
		get_canvas_point(0.2f, false),
		"to start the game",
		text_buffer);
}

void intro_interraction()
{
	if (bn::keypad::held(bn::keypad::key_type::START))
		switch_to_state(GameState::Game);
}