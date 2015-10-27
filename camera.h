#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

class camera
{
public:

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 rotation;

	void move(float forward, float side);

	DirectX::XMFLOAT4X4 view_mat() const;

	DirectX::XMFLOAT4X4 projection;
	
};

