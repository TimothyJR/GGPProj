#include "player.h"

void player::update(float dt)
{
	auto dv = DirectX::XMVectorMultiply(DirectX::XMLoadFloat3(&this->acceleration), DirectX::XMVectorSet(dt, dt, dt, dt));
	DirectX::XMStoreFloat3(&this->velocity, DirectX::XMVectorAdd(dv, DirectX::XMLoadFloat3(&this->velocity)));

	auto dx = DirectX::XMVectorMultiply(DirectX::XMLoadFloat3(&this->velocity), DirectX::XMVectorSet(dt, dt, dt, dt));
	DirectX::XMStoreFloat3(&this->position, DirectX::XMVectorAdd(dx, DirectX::XMLoadFloat3(&this->position)));
}

player::player(entity init)
	: entity(init), acceleration(0, 0, 0), velocity(0, 0, 0), in_air(false)
{}

player make_player(entity init) {
	return entity(init);
}