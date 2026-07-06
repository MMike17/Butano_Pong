#include "canvas.h"

int get_canvas_point(bn::fixed percent, bool is_x)
{
	int dimention = is_x ? bn::display::width() : bn::display::height();
	return (int)(dimention / 2 * ((percent - 0.5f) * 2)) * (is_x ? 1 : -1);
}

bn::fixed_point get_canvas_pos(bn::fixed x_percent, bn::fixed y_percent)
{
	return bn::fixed_point(
		get_canvas_point(x_percent, true),
		get_canvas_point(y_percent, false));
}

void place_on_canvas(bn::sprite_ptr sprite, bn::fixed x_percent, bn::fixed y_percent)
{
	sprite.set_position(get_canvas_pos(x_percent, y_percent));
	sprite.set_scale(2);
}