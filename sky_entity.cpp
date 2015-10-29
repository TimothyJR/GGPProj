#include "sky_entity.h"



sky_entity::sky_entity(mesh& mesh, material& shader, skybox_texture& texture)
	: object_mesh(mesh), shader(shader), texture(texture)
{

}

sky_entity::~sky_entity()
{
}

void sky_entity::draw(ID3D11DeviceContext& device, const camera& camera) const
{
	
	this->shader.vertex.set_data("view", camera.view_mat());
	this->shader.vertex.set_data("projection", camera.projection);
	this->shader.pixel.set_sampler_state("trilinear", this->texture.state);
	this->shader.pixel.set_shader_resource_view("sky", this->texture.resource_view);
	device.RSSetState(this->texture.rasterizer_state);
	
	device.OMSetDepthStencilState(this->texture.depth_state, 0);
	this->shader.vertex.activate(true);
	this->shader.pixel.activate(true);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	device.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device.IASetVertexBuffers(0, 1, &this->object_mesh.vertices.buffer, &stride, &offset);
	device.IASetIndexBuffer(this->object_mesh.indices.buffer, DXGI_FORMAT_R32_UINT, 0);
	device.DrawIndexed(
		this->object_mesh.indices.len,
		0,
		0);

	// reset states to default
	device.RSSetState(0);
	device.OMSetDepthStencilState(0, 0);
}

sky_entity make_sky_entity(mesh& mesh, material& shader, skybox_texture& texture) {
	return sky_entity(mesh, shader, texture);
}