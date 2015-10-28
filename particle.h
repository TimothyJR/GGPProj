#pragma once
#include <DirectXMath.h>
#include "mesh.h"
#include "camera.h"
#include "material.h"
#include "Entity.h"

struct particle {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 start_velocity;
	DirectX::XMFLOAT4 start_color;
};

class particle_container
{
public:
	using buffer = d3d_buffer<particle, D3D11_BIND_VERTEX_BUFFER>;
	friend particle_container make_particle(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 emitPosition, int particle_amount, float duration, DirectX::XMFLOAT3 start_velocity, DirectX::XMFLOAT4 start_color, material& material, ID3D11Device& device);
	void update(float dt);
	void draw(ID3D11DeviceContext& device, const camera& camera) const;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

private:
	particle_container(buffer particles, material& shader);
	void update_world_matrix() const;

	material& shader;
	int particle_amount;
	float duration;
	float dt;
	buffer particles;
	mutable DirectX::XMFLOAT4X4 world_matrix;
};


