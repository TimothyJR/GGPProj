#include "particle.h"



particle_container make_particle(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 emitPosition, int particle_amount, float duration, DirectX::XMFLOAT3 start_velocity, DirectX::XMFLOAT4 start_color, material& material, ID3D11Device& device) {

	
	auto particles = std::unique_ptr<particle[]>(new particle[particle_amount]);
	for (int i = 0; i < particle_amount; i++) {
		particles[i].position = emitPosition;
		particles[i].start_velocity = start_velocity;
		particles[i].start_color = start_color;
	}

	auto buf = make_buffer(std::move(particles), particle_amount, device).take();

	particle_container ret(std::move(buf), material);
	ret.particle_amount = particle_amount;
	ret.duration = duration;
	ret.position = position;
	ret.rotation = rotation;
	ret.scale = scale;
	ret.dt = 0;
	return ret;
}


void particle_container::update(float dt)
{
	this->dt += dt;
}

void particle_container::draw(ID3D11DeviceContext & device, const camera & camera) const
{
	this->update_world_matrix();
	this->shader.geometry->set_data("world", this->world_matrix);
	this->shader.geometry->set_data("view", camera.view_mat());
	this->shader.geometry->set_data("projection", camera.projection);
	this->shader.geometry->set_data("t", dt);
	//this->shader.pixel.set_sampler_state("state", this->shader.texture_data->state);
	//this->shader.pixel.set_shader_resource_view("res", this->shader.texture_data->resource_view);
	this->shader.vertex.activate(true);
	this->shader.geometry->activate(true);
	this->shader.pixel.activate(true);
	UINT stride = sizeof(particle);
	UINT offset = 0;
	device.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	device.IASetVertexBuffers(0, 1, &this->particles.buffer, &stride, &offset);
	//device.IASetIndexBuffer(this->object_mesh.indices.buffer, DXGI_FORMAT_R32_UINT, 0);
	device.Draw(this->particle_amount, 0);

	geometry_shader::deactivate(device);
}

void particle_container::update_world_matrix() const
{
	auto translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&this->position));
	auto axis = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&this->rotation));
	auto angle = DirectX::XMVector3Length(DirectX::XMLoadFloat3(&this->rotation));
	auto rotation = DirectX::XMMatrixRotationNormal(axis, angle.m128_f32[0]);
	auto scale = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&this->scale));

	auto total = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(scale, rotation), translation);
	DirectX::XMStoreFloat4x4(&world_matrix, DirectX::XMMatrixTranspose(total));
}

particle_container::particle_container(particle_container::buffer particles, material& shader)
	: particles(particles), shader(shader)
{
}

