#include <bn_fixed_point.h>
#include <bn_math.h>
#include <cmath>
#include "vector2.h"

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

	const bn::fixed_point normalize(bn::fixed_point vector)
	{
		return vector /= bn::sqrt(vector.x() * vector.x() + vector.y() * vector.y());
	}

	const bn::fixed angle(bn::fixed_point from, bn::fixed_point to)
	{
		return acos(bn::clamp<bn::fixed>(dot(normalize(from), normalize(to)), -1, 1).to_double()) * 57.29578f;
	}

	const bn::fixed dot(bn::fixed_point from, bn::fixed_point to)
	{
		return from.x() * to.x() + from.y() * to.y();
	}
}