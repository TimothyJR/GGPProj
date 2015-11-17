#pragma once
#include "Material.h"
#include "Camera.h"


struct shadow_map_texture {
	dxi_ptr<ID3D11DepthStencilView> depth_stencil_view;
	dxi_ptr<ID3D11RasterizerState> rasterizer_state;
	dxi_ptr<ID3D11ShaderResourceView> resource_view;
	dxi_ptr<ID3D11SamplerState> sampler_state;
	friend shadow_map_texture make_shadow_map_texture(dx_info& info, int size);
private:
	shadow_map_texture();
};

struct light_info {
	// these should be matrices, not vectors?
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

class shadow_map
{
public:
	~shadow_map();
	void activate(dx_info& info, const light_info& light);
	void deactivate(dx_info& info);
	vertex_shader& shader;
	shadow_map_texture shadow;
	DirectX::XMFLOAT4X4 light_view;
	DirectX::XMFLOAT4X4 light_projection;

	friend shadow_map make_shadow_map(dx_info& info, vertex_shader& shader, int size);

	const int shadow_map_size;
private: 
	shadow_map(vertex_shader& shader, shadow_map_texture shadow, int size);
};

