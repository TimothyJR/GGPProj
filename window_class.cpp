#include "window_class.h"
#include "debug.h"

window make_window(HWND handle)
{
	window ret;
	ret.handle = handle;
	return ret;
}

WNDCLASSEX register_window_class(const wchar_t* class_name, WNDPROC proc_handler, HINSTANCE app_handle)
{
	// this struct holds information for the window class
	WNDCLASSEX wc;

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = proc_handler;
	wc.hInstance = app_handle;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = class_name;

	// register the window class
	RegisterClassEx(&wc);

	return wc;
}

dx_info make_dx_info(const window& window);

window create_window(const wchar_t* class_name, const wchar_t* window_title, HINSTANCE app_handle, int show_command)
{
	// create the window and use the result as the handle
	auto window = make_window(CreateWindowEx(NULL,
		class_name,    // name of the window class
		window_title,   // title of the window
		WS_OVERLAPPEDWINDOW,    // window style
		0,    // x-position of the window
		0,    // y-position of the window
		0,    // width of the window
		0,    // height of the window
		NULL,    // we have no parent window
		NULL,    // we aren't using menus
		app_handle,   // application handle
		NULL));    // used with multiple windows
	ShowWindow(window.handle, show_command);
	window.dx = make_dx_info(window);

	return window;
}

void window::set_size(int32_t width, int32_t  height)
{
	RECT window_rectangle = { 0, 0, width, height };
	SetWindowPos(this->handle, NULL, window_rectangle.top, window_rectangle.left, width, height, 0);
	AdjustWindowRect(&window_rectangle, WS_OVERLAPPEDWINDOW, false);
	this->dx.resize(width, height);
}

dx_info make_dx_info(const window& window) 
{
	dx_info ret{ 0 };

	ret.driver_type = D3D_DRIVER_TYPE_HARDWARE;
	ret.feature_level = D3D_FEATURE_LEVEL_11_0;

	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	RECT window_size;
	GetWindowRect(window.handle, &window_size);

	ret.aspect_ratio = (float)(window_size.right - window_size.left) / (window_size.bottom - window_size.top);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = window_size.right - window_size.left;
	swapChainDesc.BufferDesc.Height = window_size.bottom - window_size.top;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = window.handle;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Create the device and swap chain with the following global function
	// This will also determine the supported feature level (version of DirectX)
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		0,
		ret.driver_type,
		0,
		createDeviceFlags,
		0,
		0,
		D3D11_SDK_VERSION,
		&swapChainDesc,		// We pass in the description
		&ret.swap_chain,			// And the SWAP CHAIN is created
		&ret.device,			// As well as the DEVICE
		&ret.feature_level,		// The feature level is determined
		&ret.device_context);	// And the CONTEXT is created

							// Was the device created properly?
	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
	}

	return ret;
}

#define ReleaseMacro(x) { if(x){ x->Release(); x = 0; } }

void dx_info::resize(int32_t width, int32_t height)
{
	
	// Release any existing views, since we'll be destroying
	// the corresponding buffers.
	ReleaseMacro(this->render_target_view);
	ReleaseMacro(this->depth_stencil_view);
	ReleaseMacro(this->depth_stencil_buffer);

	// Resize the swap chain to match the new window dimensions
	HR(this->swap_chain->ResizeBuffers(
		1,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0));

	// Recreate the render target view that points to the swap chain's buffer
	ID3D11Texture2D* backBuffer;
	HR(this->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(this->device->CreateRenderTargetView(backBuffer, 0, &this->render_target_view));
	ReleaseMacro(backBuffer);

	// Set up the description of the texture to use for the depth buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	// Create the depth/stencil buffer and corresponding view
	HR(this->device->CreateTexture2D(&depthStencilDesc, 0, &this->depth_stencil_buffer));
	HR(this->device->CreateDepthStencilView(this->depth_stencil_buffer, 0, &this->depth_stencil_view));

	// Bind these views to the pipeline, so rendering properly 
	// uses the underlying textures
	this->device_context->OMSetRenderTargets(1, &this->render_target_view, this->depth_stencil_view);

	// Update the viewport to match the new window size and set it on the device
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	this->device_context->RSSetViewports(1, &viewport);

	// Recalculate the aspect ratio, since it probably changed
	this->aspect_ratio = (float)width / height;
	
}