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
	DirectX::XMFLOAT4 end_color;
};

class particle_container
{
public:
	using buffer = d3d_buffer<particle, D3D11_BIND_VERTEX_BUFFER>;
	friend particle_container make_particle(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 emitPosition, int particle_amount, float duration, float start_speed, float end_speed, float start_size, float end_size, float angle_max, DirectX::XMFLOAT4 start_color, DirectX::XMFLOAT4 end_color, int full_sphere, material& material, texture& texture, ID3D11Device& device);
	void update(float dt);
	void draw(ID3D11DeviceContext& device, const camera& camera) const;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	float duration;
	float dt;

private:
	particle_container(buffer particles, material& shader, texture& texture);
	void update_world_matrix() const;

	material& shader;
	texture& shader_texture;
	int particle_amount;
	
	
	float start_speed;
	float end_speed;
	float start_size;
	float end_size;
	buffer particles;
	mutable DirectX::XMFLOAT4X4 world_matrix;
};


