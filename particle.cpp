#include "particle.h"
#include <math.h>


particle_container make_particle(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 emitPosition, int particle_amount, float duration, float start_speed, float end_speed, float start_size, float end_size, float rotation_over_time, bool randomize_start_rotation, float angle_max, DirectX::XMFLOAT4 start_color, DirectX::XMFLOAT4 end_color, int full_sphere, material& material, texture& texture, ID3D11Device& device) {

	auto particles = std::unique_ptr<particle[]>(new particle[particle_amount]);
	for (int i = 0; i < particle_amount; i++) {
		auto randomRadius = (float)rand() / (float)RAND_MAX * 2 - 1;
		auto randomAngle = (float)rand() / (float)RAND_MAX * angle_max;
		auto randomHeight = (float)rand() / (float)RAND_MAX * (1 + full_sphere) - (1 * full_sphere);
		auto direction = DirectX::XMFLOAT3(randomRadius * cos(randomAngle), randomHeight, randomRadius * sin(randomAngle));

		float startRotation = 0.0f;
		if (randomize_start_rotation)
		{
			startRotation = (float)rand() / (float)RAND_MAX * 6.29f;
		}

		// Set the magnitude of the vector to the speed
		auto directionMagSquared = sqrt(pow(direction.x, 2) + pow(direction.y, 2) + pow(direction.z, 2));
		direction = DirectX::XMFLOAT3((direction.x / directionMagSquared) * start_speed, (direction.y / directionMagSquared)  * start_speed, (direction.z / directionMagSquared)  * start_speed);

		particles[i].position = emitPosition;
		particles[i].start_velocity = direction;
		particles[i].start_color = start_color;
		particles[i].end_color = end_color;
		particles[i].start_rotation = startRotation;
	}

	auto buf = make_buffer(std::move(particles), particle_amount, device).take();

	particle_container ret(std::move(buf), material, texture);
	ret.particle_amount = particle_amount;
	ret.duration = duration;
	ret.position = position;
	ret.rotation = rotation;
	ret.scale = scale;
	ret.start_speed = start_speed;
	ret.end_speed = end_speed;
	ret.start_size = start_size;
	ret.end_size = end_size;
	ret.rotation_change = rotation_over_time;
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
	this->shader.geometry->set_data("rotation_change", rotation_change);
	this->shader.geometry->set_data("duration", duration);
	this->shader.geometry->set_data("start_speed", start_speed);
	this->shader.geometry->set_data("end_speed", end_speed);
	this->shader.geometry->set_data("start_size", start_size);
	this->shader.geometry->set_data("end_size", end_size);
	this->shader.pixel.set_sampler_state("state", this->shader_texture.state);
	this->shader.pixel.set_shader_resource_view("res", this->shader_texture.resource_view);
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

particle_container::particle_container(particle_container::buffer particles, material& shader, texture& texture)
	: particles(particles), shader(shader), shader_texture(texture)
{
}

