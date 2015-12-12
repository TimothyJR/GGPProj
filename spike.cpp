#include "spike.h"

spike make_spike(enemy init)
{
	return enemy(init);
}

void spike::update(float dt)
{}

spike::spike(enemy init) : enemy(init)
{
	m_half_width = 0.3f;
	m_half_height = 0.3f;
}