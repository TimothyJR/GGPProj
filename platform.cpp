#include "platform.h"
#include "my_math.h"

platform make_platform(entity model, float width, float height)
{
	auto ret = platform(model);
	ret.m_half_width = width / 2.0f;
	ret.m_half_height = height / 2.0f;
	return ret;
}

option<collision_result> platform::collides_with(const player & player) const
{
	auto x_diff = player.position.x - this->position.x;
	auto y_diff = player.position.y - this->position.y;

	auto player_half_width = 0.5f;
	auto player_half_height = 0.3f;

	auto half_width_sum = this->m_half_width + player_half_width;
	auto half_height_sum = this->m_half_height + player_half_height;

	collision_result ret;
	ret.penetration.x = half_width_sum - abs(x_diff);
	ret.penetration.y = half_height_sum - abs(y_diff);

	if (ret.penetration.x > 0 && ret.penetration.y > 0) {
		ret.penetration.x *= sign_of(x_diff);
		ret.penetration.y *= sign_of(y_diff);
		return Some(ret);
	}
	else {
		return None<collision_result>();
	}
}

platform::platform(entity e)
	: entity(e)
{
}
