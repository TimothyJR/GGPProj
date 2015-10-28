#include "texture.h"
#include "debug.h"
#include <WICTextureLoader.h>
option<texture> load_texture_from_file(const wchar_t * filename, ID3D11Device& device)
{

	texture ret;
	auto result = DirectX::CreateWICTextureFromFile(&device, filename, nullptr, &ret.resource_view);
	if (result != S_OK) {
		return None<texture>();
	}

	D3D11_SAMPLER_DESC builder;
	ZeroMemory(&builder, sizeof(builder));
	builder.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	builder.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	builder.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	builder.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	builder.MaxLOD = D3D11_FLOAT32_MAX;
	result = device.CreateSamplerState(&builder, &ret.state);

	if (result != S_OK) {
		return None<texture>();
	}

	return Some(std::move(ret));
}

texture::texture()
{
}
