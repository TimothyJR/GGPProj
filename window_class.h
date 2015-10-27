#pragma once
#include <Windows.h>
#include <stdint.h>
#include <d3d11.h>
#include "dxi_ptr.h"
struct dx_info 
{
public:
	dxi_ptr<ID3D11Device> device;
	dxi_ptr<ID3D11DeviceContext> device_context;
	dxi_ptr<IDXGISwapChain> swap_chain;
	dxi_ptr<ID3D11Texture2D> depth_stencil_buffer;
	dxi_ptr<ID3D11RenderTargetView> render_target_view;
	dxi_ptr<ID3D11DepthStencilView> depth_stencil_view;
	D3D11_VIEWPORT viewport;
	D3D_DRIVER_TYPE driver_type;
	D3D_FEATURE_LEVEL feature_level;

	float aspect_ratio;

	void resize(int32_t width, int32_t height);
};

struct window
{
public:
	HWND handle;
	dx_info dx;

	void set_size(int32_t width, int32_t height);
};

WNDCLASSEX register_window_class(const wchar_t* class_name, WNDPROC proc_handler, HINSTANCE app_handle);

window create_window(const wchar_t* class_name, const wchar_t* window_title, HINSTANCE app_handle, int show_command);