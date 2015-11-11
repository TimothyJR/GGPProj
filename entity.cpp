#include "Entity.h"


entity::entity(mesh& mesh, material& shader, texture& texture)
: object_mesh(mesh), shader(shader), shader_texture(texture)
{
	DirectX::XMMATRIX iden = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4(&world_matrix, iden);
	position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

}

entity::~entity()
{
}



void entity::update_world_matrix() const
{
	auto translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&this->position));
	auto axis = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&this->rotation));
	auto angle = DirectX::XMVector3Length(DirectX::XMLoadFloat3(&this->rotation));
	auto rotation = DirectX::XMMatrixRotationNormal(axis, angle.m128_f32[0]);
	auto scale = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&this->scale));
	
	auto total = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(scale, rotation), translation);
	DirectX::XMStoreFloat4x4(&world_matrix, DirectX::XMMatrixTranspose(total));
}

void entity::draw(ID3D11DeviceContext& device, const camera& camera) const
{
	this->update_world_matrix();

	this->shader.vertex.set_data("world", this->world_matrix);
	this->shader.vertex.set_data("view", camera.view_mat());
	this->shader.vertex.set_data("projection", camera.projection);
	this->shader.pixel.set_sampler_state("state", this->shader_texture.state);
	this->shader.pixel.set_shader_resource_view("res", this->shader_texture.resource_view);
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

}

void entity::draw_with_activated_shader(ID3D11DeviceContext& device, vertex_shader& activated_vertex_shader) const
{ 
	this->update_world_matrix();

	activated_vertex_shader.set_data("world", this->world_matrix);
	activated_vertex_shader.copy_all_buffers();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	device.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device.IASetVertexBuffers(0, 1, &this->object_mesh.vertices.buffer, &stride, &offset);
	device.IASetIndexBuffer(this->object_mesh.indices.buffer, DXGI_FORMAT_R32_UINT, 0);
	device.DrawIndexed(
		this->object_mesh.indices.len,
		0,
		0);
}

entity make_entity(mesh& mesh, material& shader, texture& texture) {
	return entity(mesh, shader, texture);
}