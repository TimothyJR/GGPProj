#pragma once
#include "Material.h"
#include "Mesh.h"
#include "Camera.h"
class sky_entity
{
public:
	~sky_entity();
	void draw(ID3D11DeviceContext& device, const camera& camera) const;
	mesh& object_mesh;
	material& shader;

	friend sky_entity make_sky_entity(mesh& mesh, material& shader);
private:
	sky_entity(mesh& mesh, material& shader);
};

