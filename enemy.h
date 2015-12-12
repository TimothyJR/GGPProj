#pragma once
#include "Entity.h"
#include "player.h"
#include "option.h"

struct enemy_collision_result {
	struct {
		float x;
		float y;
	} penetration;
};

struct enemy : public entity
{
	friend enemy make_enemy(entity);
	option<enemy_collision_result> collides_with(const player& player) const;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 acceleration;
	bool active;
	bool in_air;

	void setActive(bool isActive);
	virtual void update(float dt);

private:
	enemy(entity);

protected:
	float m_half_width;
	float m_half_height;
};

