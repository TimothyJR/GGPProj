#pragma once
#include <d3d11.h>
#include "option.h"
#include "dxi_ptr.h"

struct texture {

	friend option<texture> load_texture_from_file(const wchar_t* filename, ID3D11Device& device);
	dxi_ptr<ID3D11ShaderResourceView> resource_view;
	dxi_ptr<ID3D11SamplerState> state;
private:
	texture();
};

