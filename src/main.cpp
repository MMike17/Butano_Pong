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
#include <bn_timer.h>
#include <bn_timers.h>
#include <bn_string.h>
#include <bn_sstream.h>
#include <bn_rect.h>

// custom imports
#include "main.h"
#include "canvas.h"

// sprite imports
#include <bn_sprite_items_common_fixed_8x8_font.h>
#include <bn_sprite_items_paddle.h>
#include <bn_sprite_items_ball.h>

const int PLAYER_SPEED{2};
const int PADDLE_WIDTH{4};
const int BALL_SIZE{4};
const int BALL_SPEED{1};
const int SCREEN_X_LIMIT{bn::display::width() / 2};
const int SCREEN_Y_LIMIT{bn::display::height() / 2};
const int SCORE_ANIM_DURATION{2};
const int SCORE_ANIM_FLASHES{2};
const bn::fixed SCORE_MODULO{SCORE_ANIM_FLASHES / SCORE_ANIM_FLASHES * 0.5f};
const bn::fixed SCORE_ANIM_RATIO{1 / SCORE_MODULO}; // I can't modulo with floats...but I can divide the timer by modulo
const bn::sprite_font FONT(bn::sprite_items::common_fixed_8x8_font);

GameState state;
bn::vector<bn::sprite_ptr, 32> text_buffer;
bn::optional<bn::sprite_ptr> player_palette;
bn::optional<bn::sprite_ptr> ai_palette;
bn::optional<bn::sprite_ptr> ball;
bn::optional<bn::rect> palette_rect;
bn::optional<bn::rect> ball_rect;
bn::fixed_point player_pos;
bn::fixed_point ai_pos;
bn::fixed_point ball_pos;
bn::fixed_point ball_velocity;
bn::random random;
bn::timer score_anim_timer;
int paddle_offset;
int paddle_y_limit;
int player_score{0};
int ai_score{0};
bool waiting_for_input;
bool score_anim;
bool is_player_point;

// TODO : Reset paddle pos before restart prompt
// TODO : Fix score anim flashing player when ai scores
// TODO : Fix error on finish game
// TODO : move ai palette

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
		text_buffer.clear();
		display_text(get_canvas_pos(0.5f, 0.5f), player_score == 10 ? "Player wins !" : "AI wins !");

		if (bn::keypad::pressed(bn::keypad::key_type::A))
		{
			player_score = 0;
			ai_score = 0;
			switch_to_state(GameState::Game);
		}
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
	case GameState::Game:
	{
		player_palette.reset();
		ai_palette.reset();
		ball.reset();
		break;
	}

	default:
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
	text_buffer.clear();
	display_text(get_canvas_pos(0.5f, 0.7f), "Pong");
	display_text(get_canvas_pos(0.5f, 0.3f), "Press [start]");
	display_text(get_canvas_pos(0.5f, 0.2f), "to start the game");
}

void game_init()
{
	waiting_for_input = true;

	// set initial positions
	const bn::sprite_shape_size paddle_size{bn::sprite_items::paddle.shape_size()};
	paddle_offset = paddle_size.width() / 2 - PADDLE_WIDTH / 2;

	player_pos = get_canvas_pos(0.1f, 0.5f);
	player_pos.set_x(player_pos.x() + paddle_offset);
	ai_pos = get_canvas_pos(0.9f, 0.5f);
	ai_pos.set_x(ai_pos.x() + paddle_offset);
	ball_pos = get_canvas_pos(0.5f, 0.5f);

	paddle_y_limit = bn::display::height() / 2 - paddle_size.height() / 2;

	// spawn sprites
	player_palette = bn::sprite_items::paddle.create_sprite_optional(player_pos);
	ai_palette = bn::sprite_items::paddle.create_sprite_optional(ai_pos);
	palette_rect = bn::rect(0, 0, PADDLE_WIDTH, paddle_size.height());
	ball = bn::sprite_items::ball.create_sprite_optional(ball_pos);
	ball_rect = bn::rect(0, 0, BALL_SIZE, BALL_SIZE);
}

void intro_logic()
{
	if (bn::keypad::pressed(bn::keypad::key_type::START))
		switch_to_state(GameState::Game);
}

void game_logic()
{
	text_buffer.clear();
	bn::string<5> score_display;
	bn::ostringstream builder{score_display};
	builder.append_args(player_score, " / ", ai_score);

	if (score_anim)
	{
		// I tried making a timer cast to bn::fixed but it looped weirdly (0 -> 2 -> -2 -> 0)
		bn::fixed timer = (float)score_anim_timer.elapsed_ticks() / bn::timers::ticks_per_second();
		bn::fixed warped_timer = timer * SCORE_ANIM_RATIO; // timer divided by modulo
		int value = (int)((warped_timer - (warped_timer % 1)) / 1);
		bool show_score = value % 2 == 1; // strict sin

		bn::string player_score_display = bn::to_string<1>(player_score);
		bn::string ai_score_display = bn::to_string<1>(ai_score);

		builder.str().clear();
		builder.append_args(
			player_score ? (show_score ? " " : player_score_display) : player_score_display,
			" / ",
			!player_score ? (show_score ? " " : ai_score_display) : ai_score_display);

		if (timer >= SCORE_ANIM_DURATION)
		{
			score_anim = false;
			waiting_for_input = true;

			player_pos.set_y(0);
			ai_pos.set_y(0);
			ball_pos = bn::fixed_point(0, 0);
			ball_velocity = bn::fixed_point(0, 0);
			ball.value().set_position(ball_pos);
		}
	}
	else if (waiting_for_input)
	{
		display_text(get_canvas_pos(0.5f, 0.7f), "Press [A] to start the game");

		if (bn::keypad::pressed(bn::keypad::key_type::A))
		{
			// normalize close to 1 is okay
			bn::fixed random_x = random.get_fixed(-1, 1);
			bn::fixed random_y = random.get_fixed(-1, 1);
			bn::fixed length = bn::sqrt((random_x * random_x) + (random_y * random_y));

			ball_velocity = bn::fixed_point(random_x / length, -random_y / length) * BALL_SPEED;
			waiting_for_input = false;
		}
	}
	else
	{
		if (bn::keypad::held(bn::keypad::key_type::UP))
			player_pos.set_y(player_pos.y() - PLAYER_SPEED);

		if (bn::keypad::held(bn::keypad::key_type::DOWN))
			player_pos.set_y(player_pos.y() + PLAYER_SPEED);

		// clamp player pos to screen
		player_pos.set_y(bn::min<bn::fixed>(player_pos.y(), paddle_y_limit));
		player_pos.set_y(bn::max<bn::fixed>(player_pos.y(), -paddle_y_limit));

		player_palette.value().set_position(player_pos);
		ball.value().set_position(ball_pos += ball_velocity);
		ball_collisions();
	}

	display_text(get_canvas_pos(0.5f, 0.95f), score_display);
}

void display_text(bn::fixed_point pos, bn::string_view text)
{
	bn::sprite_text_generator text_display{FONT};
	text_display.set_center_alignment();
	text_display.generate(pos, text, text_buffer);
}

void ball_collisions()
{
	palette_rect.value().set_position((int)player_pos.x() - paddle_offset + PADDLE_WIDTH / 2, (int)player_pos.y());
	ball_rect.value().set_position((int)ball_pos.x(), (int)ball_pos.y());

	if (ball_velocity.x() < 0 && palette_rect.value().intersects(ball_rect.value()))
		ball_velocity.set_x(-ball_velocity.x());

	// TODO : fix rect detection for ai ?
	palette_rect.value().set_position((int)ai_pos.x() - PADDLE_WIDTH / 2, (int)ai_pos.y());

	if (ball_velocity.x() > 0 && palette_rect.value().intersects(ball_rect.value()))
		ball_velocity.set_x(-ball_velocity.x());

	if (ball_pos.y() + BALL_SIZE / 2 >= SCREEN_Y_LIMIT || ball_pos.y() - BALL_SIZE / 2 <= -SCREEN_Y_LIMIT)
		ball_velocity.set_y(-ball_velocity.y());

	is_player_point = ball_pos.x() > SCREEN_X_LIMIT;

	if (is_player_point || ball_pos.x() < -SCREEN_X_LIMIT)
	{
		if (is_player_point)
			++player_score;
		else
			++ai_score;

		score_anim = true;
		score_anim_timer.restart();
	}
}