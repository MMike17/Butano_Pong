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
#include <bn_math.h>
#include <bn_random.h>
#include <bn_fixed.h>
#include <bn_math.h>

// custom imports
#include "main.h"
#include "canvas.h"

// sprite imports
#include <bn_sprite_items_common_fixed_8x8_font.h>
#include <bn_sprite_items_paddle.h>
#include <bn_sprite_items_ball.h>

const int PLAYER_SPEED{2};
const int PADDLE_WIDTH{4};
const int BALL_SPEED{1};
const int SCREEN_X_LIMIT{bn::display::width() / 2};
const int SCREEN_Y_LIMIT{bn::display::height() / 2};
const bn::sprite_font FONT(bn::sprite_items::common_fixed_8x8_font);

GameState state;
bn::vector<bn::sprite_ptr, 32> text_buffer;
bn::optional<bn::sprite_ptr> player_palette;
bn::optional<bn::sprite_ptr> ai_palette;
bn::optional<bn::sprite_ptr> ball;
bn::fixed_point player_pos;
bn::fixed_point ai_pos;
bn::fixed_point ball_pos;
bn::fixed_point ball_velocity;
bn::random random;
int paddle_y_limit;
bool paused;

void init()
{
	state = GameState::Intro;
	bn::core::init();

	// sets background color for debug
	bn::bg_palettes::set_transparent_color(bn::color(2, 2, 2));
}

int main()
{
	init();
	switch_to_state(GameState::Intro);

	while (true)
	{
		state_update();
		random.update();
		bn::core::update();
	}
}

void state_update()
{
	switch (state)
	{
	case GameState::Intro:
		intro_logic();
		break;

	case GameState::Game:
		game_logic();
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
	{
		text_buffer.clear();
		break;
	}

	case GameState::Game:
	{
		player_palette.reset();
		ai_palette.reset();
		ball.reset();
		break;
	}

	default: // this should never happen
		break;
	}

	state = newState;

	// init
	switch (newState)
	{
	case GameState::Intro:
		intro_init();
		break;

	case GameState::Game:
		game_init();
		break;

	default:
		break;
	}

	BN_LOG("Switched to state : ", GameStateStrings[newState]);
}

void intro_init()
{
	bn::sprite_text_generator text_generator(FONT);
	text_generator.set_center_alignment();
	text_buffer.clear();

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

void game_init()
{
	paused = true;

	// set initial positions
	const bn::sprite_shape_size paddle_size{bn::sprite_items::paddle.shape_size()};
	const int paddle_offset{paddle_size.width() / 2 - PADDLE_WIDTH / 2};

	player_pos = get_canvas_pos(0.1f, 0.5f);
	player_pos.set_x(player_pos.x() + paddle_offset);
	ai_pos = get_canvas_pos(0.9f, 0.5f);
	ai_pos.set_x(ai_pos.x() + paddle_offset);
	ball_pos = get_canvas_pos(0.5f, 0.5f);

	paddle_y_limit = bn::display::height() / 2 - paddle_size.height() / 2;

	// spawn sprites
	player_palette = bn::sprite_items::paddle.create_sprite_optional(player_pos);
	ai_palette = bn::sprite_items::paddle.create_sprite_optional(ai_pos);
	ball = bn::sprite_items::ball.create_sprite_optional(ball_pos);
}

void intro_logic()
{
	if (bn::keypad::held(bn::keypad::key_type::START))
		switch_to_state(GameState::Game);
}

void game_logic()
{
	// TODO : Bounce ball on paddles
	// TODO : Display points

	if (paused)
	{
		if (bn::keypad::pressed(bn::keypad::key_type::A))
		{
			// normalize close to 1 is okay
			bn::fixed random_x = random.get_fixed(-1, 1);
			bn::fixed random_y = random.get_fixed(-1, 1);
			bn::fixed length = bn::sqrt((random_x * random_x) + (random_y * random_y));

			ball_velocity = bn::fixed_point(random_x / length, -random_y / length) * BALL_SPEED;
			paused = false;
		}
	}
	else
	{
		if (bn::keypad::held(bn::keypad::key_type::UP))
			player_pos.set_y(player_pos.y() - PLAYER_SPEED);

		if (bn::keypad::held(bn::keypad::key_type::DOWN))
			player_pos.set_y(player_pos.y() + PLAYER_SPEED);

		player_pos.set_y(bn::min<bn::fixed>(player_pos.y(), paddle_y_limit));
		player_pos.set_y(bn::max<bn::fixed>(player_pos.y(), -paddle_y_limit));

		player_palette.value().set_position(player_pos);
		ball.value().set_position(ball.value().position() + ball_velocity);

		bool is_player_point = ball_pos.x() > SCREEN_X_LIMIT;
		bool is_ai_point = ball_pos.x() < -SCREEN_X_LIMIT;

		if (is_player_point || is_ai_point)
		{
			// TODO : Interrupt and animate point scoring
			// TODO : reset paddles
		}
	}
}