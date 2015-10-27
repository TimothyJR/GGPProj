#pragma once
#include "Entity.h"
#include "player.h"
#include "option.h"
/*
changing the scale values of this entity does not change the width or the height in terms of collisions
*/

struct collision_result {
	struct {
		float x;
		float y;
	} penetration;
};

struct platform : public entity {
	
	friend platform make_platform(entity model, float width, float height);

	option<collision_result> collides_with(const player& player) const;

private:
	float m_half_width;
	float m_half_height;
	platform(entity);
};