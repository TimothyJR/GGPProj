#include "enemy.h"
#include "my_math.h"

enemy make_enemy(entity init)
{
	return entity(init);
}

option<enemy_collision_result> enemy::collides_with(const player & player) const
{
	auto x_diff = player.position.x - this->position.x;
	auto y_diff = player.position.y - this->position.y;

	auto player_half_width = 0.5f;
	auto player_half_height = 0.5f;

	auto half_width_sum = m_half_width + player_half_width;
	auto half_height_sum = m_half_height + player_half_height;

	enemy_collision_result ret;
	ret.penetration.x = half_width_sum - abs(x_diff);
	ret.penetration.y = half_height_sum - abs(y_diff);

	if (ret.penetration.x > 0 && ret.penetration.y > 0) {
		ret.penetration.x *= sign_of(x_diff);
		ret.penetration.y *= sign_of(y_diff);
		return Some(ret);
	}
	else {
		return None<enemy_collision_result>();
	}
}

void enemy::setActive(bool isActive)
{
	active = isActive;
}

void enemy::update(float dt)
{}

enemy::enemy(entity init)
	: entity(init), acceleration(0, 0, 0), velocity(0, 0, 0), active(true), in_air(false)
{}