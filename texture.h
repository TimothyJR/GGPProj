#pragma once
#include <d3d11.h>
#include "option.h"
#include "dxi_ptr.h"

struct texture {

	friend option<texture> load_texture_from_file(const wchar_t* filename, ID3D11Device& device);
	dxi_ptr<ID3D11ShaderResourceView> resource_view;
	dxi_ptr<ID3D11SamplerState> state;
protected:
	texture();
};


struct skybox_texture : public texture {

	friend option<skybox_texture> load_skybox(const wchar_t* filename, ID3D11Device& device);
	dxi_ptr<ID3D11RasterizerState> rasterizer_state;
	dxi_ptr<ID3D11DepthStencilState> depth_state;
private:
	skybox_texture();
};
// skybox - not sure if can combine with texture above so don't need another srv or sampler?
/*
struct skybox {

	friend option<skybox> load_skybox(const wchar_t* filename, ID3D11Device& device);
	dxi_ptr<ID3D11ShaderResourceView> resource_view;
	dxi_ptr<ID3D11SamplerState> state;
	dxi_ptr<ID3D11RasterizerState> rasterizer_state;
	dxi_ptr<ID3D11DepthStencilState> depth_state;

private:
	skybox();
};
*/

