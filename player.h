#pragma once
#include "Entity.h"


struct player : public entity {

	friend player make_player(entity);
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 acceleration;
	bool in_air;

	void update(float dt);
private:
	player(entity);
};