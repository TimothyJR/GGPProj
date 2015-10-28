#include "texture.h"
#include "debug.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
option<texture> load_texture_from_file(const wchar_t * filename, ID3D11Device& device)
{

	texture ret;
	auto result = DirectX::CreateWICTextureFromFile(&device, L"demo1.jpg", nullptr, &ret.resource_view);
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

option<texture> load_skybox(const wchar_t * filename, ID3D11Device& device)
{

	texture sky;
	auto result = DirectX::CreateDDSTextureFromFile(&device, L"SunnyCubeMap.dds", nullptr, &sky.resource_view);
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
	result = device.CreateSamplerState(&builder, &sky.state);

	// Create a rasterizer state for the sky box
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(rastDesc));
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.DepthClipEnable = true;
	device.CreateRasterizerState(&rastDesc, &sky.rasterizer_state);

	// A depth state for the sky rendering
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device.CreateDepthStencilState(&dsDesc, &sky.depth_state);

	if (result != S_OK) {
		return None<texture>();
	}

	return Some(std::move(sky));
}

texture::texture()
{
}
