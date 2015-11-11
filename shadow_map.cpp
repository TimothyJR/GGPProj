#include "shadow_map.h"



shadow_map::shadow_map(vertex_shader& shader, shadow_map_texture shadow, int size)
	: shader(shader), shadow(shadow), shadow_map_size(size)
{

}

shadow_map::~shadow_map()
{
}

void shadow_map::activate(dx_info& info, const light_info& light) 
{
	// need to set special vertex data
	// need to set basic ps and vs
	// needs to be able to draw everything to it

	info.device_context->OMSetRenderTargets(0, 0, shadow.depth_stencil_view);
	info.device_context->ClearDepthStencilView(shadow.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
	info.device_context->RSSetState(shadow.rasterizer_state);

	// need viewport
	D3D11_VIEWPORT shadow_viewport = info.viewport; // need original viewport
	shadow_viewport.Width = (float)this->shadow_map_size;
	shadow_viewport.Height = (float)this->shadow_map_size;
	info.device_context->RSSetViewports(1, &shadow_viewport);

	// turn on correct shaders
	this->shader.activate(false);
	this->shader.set_data("view", light.view); // need light matrices
	this->shader.set_data("projection", light.projection);
	info.device_context->PSSetShader(0, 0, 0); 


}

void shadow_map::deactivate(dx_info& info)
{
	// reset
	info.device_context->OMSetRenderTargets(0, nullptr, info.depth_stencil_view);
	info.device_context->RSSetViewports(1, &info.viewport);
	info.device_context->RSSetState(0);
}

shadow_map_texture make_shadow_map_texture(dx_info& info, int size)
{
	shadow_map_texture ret;
	// Create all the shadow DX stuff
	D3D11_TEXTURE2D_DESC shadow_map_description;
	shadow_map_description.Width = size;
	shadow_map_description.Height = size;
	shadow_map_description.ArraySize = 1;
	shadow_map_description.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadow_map_description.CPUAccessFlags = 0;
	shadow_map_description.Format = DXGI_FORMAT_R32_TYPELESS;
	shadow_map_description.MipLevels = 1;
	shadow_map_description.MiscFlags = 0;
	shadow_map_description.SampleDesc.Count = 1;
	shadow_map_description.SampleDesc.Quality = 0;
	shadow_map_description.Usage = D3D11_USAGE_DEFAULT;
	dxi_ptr<ID3D11Texture2D> shadowTexture;
	info.device->CreateTexture2D(&shadow_map_description, 0, &shadowTexture);

	// Create the depth/stencil view for the shadow map
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_description = {};
	depth_stencil_view_description.Flags = 0;
	depth_stencil_view_description.Format = DXGI_FORMAT_D32_FLOAT;
	depth_stencil_view_description.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_description.Texture2D.MipSlice = 0;
	info.device->CreateDepthStencilView(shadowTexture, &depth_stencil_view_description, &ret.depth_stencil_view);

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_description = {};
	shader_resource_view_description.Format = DXGI_FORMAT_R32_FLOAT;
	shader_resource_view_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_description.Texture2D.MipLevels = 1;
	shader_resource_view_description.Texture2D.MostDetailedMip = 0;
	info.device->CreateShaderResourceView(shadowTexture, &shader_resource_view_description, &ret.resource_view);

	// Create a better sampler specifically for the shadow map
	D3D11_SAMPLER_DESC sampler_description = {};
	sampler_description.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampler_description.ComparisonFunc = D3D11_COMPARISON_LESS;
	sampler_description.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_description.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_description.BorderColor[0] = 1.0f;
	sampler_description.BorderColor[1] = 1.0f;
	sampler_description.BorderColor[2] = 1.0f;
	sampler_description.BorderColor[3] = 1.0f;
	info.device->CreateSamplerState(&sampler_description, &ret.sampler_state);

	// Create a rasterizer for the shadow creation stage (to apply a bias for us)
	D3D11_RASTERIZER_DESC rasterizer_description = {};
	rasterizer_description.FillMode = D3D11_FILL_SOLID;
	rasterizer_description.CullMode = D3D11_CULL_BACK;
	rasterizer_description.FrontCounterClockwise = false;
	rasterizer_description.DepthClipEnable = true;
	rasterizer_description.DepthBias = 1000; 
	rasterizer_description.DepthBiasClamp = 0.0f;
	rasterizer_description.SlopeScaledDepthBias = 1.0f;
	info.device->CreateRasterizerState(&rasterizer_description, &ret.rasterizer_state);

	return ret;
}

shadow_map make_shadow_map(dx_info& info, vertex_shader& shader, int size) {

	return shadow_map(shader, make_shadow_map_texture(info, size), size);
}

shadow_map_texture::shadow_map_texture()
{
}
