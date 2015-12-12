#pragma once
#include "enemy.h"

struct spike : public enemy
{
	friend spike make_spike(enemy);
	void update(float dt);

private:
	spike(enemy);
};

