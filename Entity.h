#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "shadow_map.h"

class entity
{
public:
	~entity();

	void draw(ID3D11DeviceContext& device, const camera& camera, shadow_map& shadow_map) const;
	void draw_with_activated_shader(ID3D11DeviceContext& device, vertex_shader& activated_vertex_shader) const;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	mesh& object_mesh;
	material& shader;
	texture& shader_texture;

	friend entity make_entity(mesh& mesh, material& shader, texture& texture);
protected:
	void update_world_matrix() const;
	entity(mesh& mesh, material& shader, texture& texture);
	mutable DirectX::XMFLOAT4X4 world_matrix;

};

