#pragma once
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

class entity
{
public:
	~entity();

	void draw(ID3D11DeviceContext& device, const camera& camera) const;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;
	mesh& object_mesh;
	material& shader;

	friend entity make_entity(mesh& mesh, material& shader);
protected:
	void update_world_matrix() const;
	entity(mesh& mesh, material& shader);
	mutable DirectX::XMFLOAT4X4 world_matrix;

};

