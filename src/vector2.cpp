#include <bn_fixed_point.h>
#include <bn_math.h>
#include <bn_log.h>
#include <numbers>
#include "vector2.h"
#include <bn_string.h>

namespace vector2
{
	bn::fixed_point rotate_vector(bn::fixed_point vector, int angle)
	{
		bn::fixed cos{bn::degrees_cos(angle)};
		bn::fixed sin{bn::degrees_sin(angle)};

		return bn::fixed_point{
			vector.x() * cos - vector.y() * sin,
			vector.x() * sin + vector.y() * cos};
	}
}