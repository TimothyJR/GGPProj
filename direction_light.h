#pragma once
#include <DirectXMath.h>
class directional_light
{
public:
	directional_light();
	~directional_light();
	DirectX::XMFLOAT4 ambient_color;
	DirectX::XMFLOAT4 diffuse_color;
	DirectX::XMFLOAT3 direction;
};

