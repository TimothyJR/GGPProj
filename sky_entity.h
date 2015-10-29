#pragma once
#include "Material.h"
#include "Mesh.h"
#include "Camera.h"
#include "texture.h"
class sky_entity
{
public:
	~sky_entity();
	void draw(ID3D11DeviceContext& device, const camera& camera) const;
	mesh& object_mesh;
	material& shader;
	skybox_texture& texture;

	friend sky_entity make_sky_entity(mesh& mesh, material& shader, skybox_texture& texture);
private:
	sky_entity(mesh& mesh, material& shader, skybox_texture& texture);
};

