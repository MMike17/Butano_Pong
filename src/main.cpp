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
#include <bn_timer.h>
#include <bn_timers.h>
#include <bn_string.h>
#include <bn_sstream.h>
#include <bn_rect.h>
#include <bn_sound_items.h>
#include <bn_sprite_palette_ptr.h>
#include <bn_colors.h>

// custom imports
#include "main.h"
#include "canvas.h"
#include "vector2.h"

// sprite imports
#include <bn_sprite_items_common_fixed_8x8_font.h>
#include <bn_sprite_items_paddle.h>
#include <bn_sprite_items_ball.h>

const int MAX_SCORE{10};
const int PADDLE_WIDTH{4};
const int BALL_SIZE{4};
const int PADDLE_TOUCH_OFFSET{1};
const int SCREEN_X_LIMIT{bn::display::width() / 2};
const int SCREEN_Y_LIMIT{bn::display::height() / 2};
const int SCORE_ANIM_DURATION{2};
const int SCORE_ANIM_FLASHES{2};
const int PADDLE_BOOST_THRESHOLD{1};
const int MAX_ANGLE_REBOUND{70};
const int MIN_ANGLE_REBOUND{25};
const int START_ANGLE_DEADZONE{15};
const bn::fixed BALL_BOOST_MULT{1.8f};
const bn::fixed MAX_BALL_SPEED{2.5f};
const bn::fixed MIN_BALL_SPEED{1.7f};
const bn::fixed PALETTE_SPEED{1.1f};
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
bn::fixed_point ball_dir;
bn::random random;
bn::timer score_anim_timer;
bn::fixed ball_speed;
int paddle_offset;
int paddle_y_limit;
int player_score{0};
int ai_score{0};
bool waiting_for_input;
bool score_anim;
bool is_player_point;
bool has_boost;

// TODO : Make AI feel more natural
// TODO : Add VFX ?

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
		display_text(get_canvas_pos(0.5f, 0.5f), player_score == MAX_SCORE ? "Player wins !" : "AI wins !");

		if (bn::keypad::pressed(bn::keypad::key_type::A))
		{
			bn::sound_items::btn_select.play(1);
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

	bn::fixed score_percent = bn::max(player_score, ai_score) / (MAX_SCORE - 1);
	ball_speed = MIN_BALL_SPEED + (MAX_BALL_SPEED - MIN_BALL_SPEED) * score_percent;

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
	{
		bn::sound_items::btn_select.play(1);
		switch_to_state(GameState::Game);
	}
}

void game_logic()
{
	text_buffer.clear();
	bn::string<5> score_display;
	bn::ostringstream builder{score_display};
	builder.append_args(player_score, " / ", ai_score);

	if (score_anim)
	{
		// I tried making a timer cast to bn::fixed but it looped weirdly (0 -> 2 -> -2 -> 0) type overflow ?
		bn::fixed timer = (float)score_anim_timer.elapsed_ticks() / bn::timers::ticks_per_second();
		bn::fixed warped_timer = timer * SCORE_ANIM_RATIO; // timer divided by modulo
		int value = (int)((warped_timer - (warped_timer % 1)) / 1);
		bool show_score = value % 2 == 1; // strict sin

		bn::string player_score_display = bn::to_string<1>(player_score);
		bn::string ai_score_display = bn::to_string<1>(ai_score);

		builder.str().clear();
		builder.append_args(
			is_player_point ? (show_score ? " " : player_score_display) : player_score_display,
			" / ",
			!is_player_point ? (show_score ? " " : ai_score_display) : ai_score_display);

		if (timer >= SCORE_ANIM_DURATION)
		{
			score_anim = false;
			waiting_for_input = true;

			player_pos.set_y(0);
			player_palette.value().set_position(player_pos);
			ai_pos.set_y(0);
			ai_palette.value().set_position(ai_pos);
			ball_pos = bn::fixed_point(0, 0);
			ball_dir = bn::fixed_point(0, 0);
			ball.value().set_position(ball_pos);
		}

		bn::sprite_palette_ptr palette = ball.value().palette();
		palette.set_fade(bn::colors::red, 0);
	}
	else if (waiting_for_input)
	{
		has_boost = false;
		display_text(get_canvas_pos(0.5f, 0.7f), "Press [A] to start the game");

		if (bn::keypad::pressed(bn::keypad::key_type::A))
		{
			bn::sound_items::btn_select.play(1);

			int angle{lerp(START_ANGLE_DEADZONE, 90 - START_ANGLE_DEADZONE, random.get_fixed(1))};
			int sign{random.get_fixed(-1, 1) > 0 ? 1 : -1};

			ball_dir = vector2::rotate_vector(bn::fixed_point(1, 0), angle * sign);
			waiting_for_input = false;
		}
	}
	else
	{
		// move player
		if (bn::keypad::held(bn::keypad::key_type::UP))
			player_pos.set_y(player_pos.y() - PALETTE_SPEED);

		if (bn::keypad::held(bn::keypad::key_type::DOWN))
			player_pos.set_y(player_pos.y() + PALETTE_SPEED);

		// clamp player pos to screen
		player_pos.set_y(bn::max<bn::fixed>(bn::min<bn::fixed>(player_pos.y(), paddle_y_limit), -paddle_y_limit));

		// move ai
		bn::fixed y_diff = ball_pos.y() - ai_pos.y();

		// follow ball
		if (y_diff > 0)
			ai_pos.set_y(ai_pos.y() + bn::min<bn::fixed>(y_diff, PALETTE_SPEED));
		else
			ai_pos.set_y(ai_pos.y() + bn::max<bn::fixed>(y_diff, -PALETTE_SPEED));

		ai_pos.set_y(bn::max<bn::fixed>(bn::min<bn::fixed>(ai_pos.y(), paddle_y_limit), -paddle_y_limit));

		// TODO : Should I add more AI modes ?
		// follow player (for blocking)
		// try to anticipate ball

		player_palette.value().set_position(player_pos);
		ai_palette.value().set_position(ai_pos);
		ball.value().set_position(ball_pos += ball_dir * ball_speed * (has_boost ? BALL_BOOST_MULT : 1));
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
	palette_rect.value().set_position(
		(int)player_pos.x() - paddle_offset + PADDLE_WIDTH / 2 - PADDLE_TOUCH_OFFSET,
		(int)player_pos.y());
	ball_rect.value().set_position((int)ball_pos.x(), (int)ball_pos.y());

	if (ball_dir.x() < 0 && palette_rect.value().intersects(ball_rect.value()))
		manage_ball_collision(palette_rect.value());

	palette_rect.value().set_position(
		(int)ai_pos.x() - PADDLE_WIDTH * 2 + PADDLE_TOUCH_OFFSET,
		(int)ai_pos.y());

	if (ball_dir.x() > 0 && palette_rect.value().intersects(ball_rect.value()))
		manage_ball_collision(palette_rect.value());

	if (ball_pos.y() + BALL_SIZE / 2 >= SCREEN_Y_LIMIT || ball_pos.y() - BALL_SIZE / 2 <= -SCREEN_Y_LIMIT)
	{
		ball_dir.set_y(-ball_dir.y());
		bn::sound_items::impact.play(0.5f);
	}

	is_player_point = ball_pos.x() > SCREEN_X_LIMIT;

	if (is_player_point || ball_pos.x() < -SCREEN_X_LIMIT)
	{
		if (is_player_point)
			++player_score;
		else
			++ai_score;

		if (player_score == MAX_SCORE || ai_score == MAX_SCORE)
			switch_to_state(GameState::Result);
		else
		{
			score_anim = true;
			score_anim_timer.restart();
		}
	}
}

void manage_ball_collision(const bn::rect &rect)
{
	// TODO : AI paddle collision angle is reversed
	bn::fixed y_diff{ball_pos.y() - rect.position().y()};

	// ignore invalid collisions
	if (ball_pos.x() < rect.position().x() || y_diff > palette_rect.value().height() / 2)
		return;

	bn::sound_items::impact.play(1);

	// detect boost
	has_boost = y_diff <= PADDLE_BOOST_THRESHOLD && y_diff >= -PADDLE_BOOST_THRESHOLD;

	bn::sprite_palette_ptr palette = ball.value().palette();
	palette.set_fade(bn::colors::red, has_boost ? 0.7f : 0);

	// rebound + normalize
	ball_dir.set_y(0);

	if (ball_dir.x() > 0)
		ball_dir.set_x(-1);
	else
		ball_dir.set_x(1);

	// apply paddle angle
	if (!has_boost)
	{
		bn::fixed target_angle = lerp(
			MIN_ANGLE_REBOUND,
			MAX_ANGLE_REBOUND,
			((y_diff)-PADDLE_BOOST_THRESHOLD) /
				(palette_rect.value().height() / 2 - PADDLE_BOOST_THRESHOLD));

		ball_dir = vector2::rotate_vector(ball_dir, (int)target_angle);
	}
}

const inline bn::fixed lerp(const bn::fixed min, const bn::fixed max, const bn::fixed delta)
{
	return min + ((max - min) * delta);
}