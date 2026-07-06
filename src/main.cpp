// butano imports
#include <bn_core.h>
#include <bn_display.h>
#include <bn_sprite_ptr.h>
#include <bn_keypad.h>
#include <bn_sprite_text_generator.h>
#include <bn_log.h>
#include <bn_bg_palettes.h>
#include <bn_keypad.h>
#include <bn_optional.h>

// custom imports
#include "main.h"
#include "canvas.h"

// sprite imports
#include <bn_sprite_items_common_fixed_8x8_font.h>
#include <bn_sprite_items_paddle.h>
#include <bn_sprite_items_ball.h>

const int PLAYER_SPEED{1};
const int PADDLE_WIDTH{4};

GameState state;
bn::vector<bn::sprite_ptr, 32> text_buffer;
bn::optional<bn::sprite_ptr> player_palette;
bn::optional<bn::sprite_ptr> ai_palette;
bn::optional<bn::sprite_ptr> ball;
bn::fixed_point player_pos;
bn::fixed_point ai_pos;
bn::fixed_point ball_pos;

void init()
{
	state = GameState::Intro;
	bn::core::init();

	// sets background color for debug
	bn::bg_palettes::set_transparent_color(bn::color(1, 1, 1));
}

int main()
{
	init();

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
		game_display();
		game_interraction();
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
	// cleanup
	switch (state)
	{
	case GameState::Intro:
		text_buffer = bn::vector<bn::sprite_ptr, 32>();
		break;

	default: // this should never happen
		break;
	}

	state = newState;

	// init
	switch (newState)
	{
	case GameState::Game:
	{
		const int paddle_offset{bn::sprite_items::paddle.shape_size().width() / 2 - PADDLE_WIDTH / 2};

		player_pos = get_canvas_pos(0.1f, 0.5f);
		player_pos.set_x(player_pos.x() + paddle_offset);
		ai_pos = get_canvas_pos(0.9f, 0.5f);
		ai_pos.set_x(ai_pos.x() + paddle_offset);
		ball_pos = get_canvas_pos(0.5f, 0.5f);
		break;
	}

	default:
		break;
	}

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

void game_display()
{
	if (!player_palette)
		player_palette = bn::sprite_items::paddle.create_sprite_optional(player_pos);

	if (!ai_palette)
		ai_palette = bn::sprite_items::paddle.create_sprite_optional(ai_pos);

	if (!ball)
		ball = bn::sprite_items::ball.create_sprite_optional(ball_pos);
}

void game_interraction()
{
	// TODO : Limit paddle's position to screen
	// TODO : Bounce ball on paddles
	// TODO : Start game
	// TODO : Score points

	if (bn::keypad::held(bn::keypad::key_type::UP))
		player_pos.set_y(player_pos.y() - PLAYER_SPEED);

	if (bn::keypad::held(bn::keypad::key_type::DOWN))
		player_pos.set_y(player_pos.y() + PLAYER_SPEED);

	player_palette.value().set_position(player_pos);
}