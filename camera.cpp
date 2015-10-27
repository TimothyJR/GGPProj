#include "camera.h"


void camera::move(float forward, float side) {
	DirectX::XMFLOAT3 facing;
	DirectX::XMStoreFloat3(&facing, DirectX::XMVector3Transform(
		DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
		DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, 0.0f)
	));

	DirectX::XMFLOAT3 side_dir;
	DirectX::XMStoreFloat3(&side_dir, DirectX::XMVector3Cross(
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMLoadFloat3(&facing)
	));

	this->position.x += facing.x * forward + side_dir.x * side;
	this->position.y += facing.y * forward + side_dir.y * side;
	this->position.z += facing.z * forward + side_dir.z * side;
}

DirectX::XMFLOAT4X4 camera::view_mat() const
{
	DirectX::XMFLOAT4X4 ret;
	DirectX::XMStoreFloat4x4(&ret, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookToLH(
		DirectX::XMLoadFloat3(&this->position),
		DirectX::XMVector3Transform(
			DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
			DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, 0.0f)
		),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		)));
	return ret;
}