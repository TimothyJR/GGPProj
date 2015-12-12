#include <Windows.h>
#include <d3d11.h>
#include <vector>
#include "direction_light.h"
#include "debug.h"
#include "window_class.h"
#include "camera.h"
#include "texture.h"
#include "Material.h"
#include "Entity.h"
#include "sky_entity.h"
#include "Mesh.h"
#include "player.h"
#include "platform.h"
#include "my_math.h"
#include "particle.h"
#include "shadow_map.h"
#include "enemy.h"
#include "spike.h"

void update(float dt, bool& done, camera& camera, std::vector<platform>& platforms, std::vector<enemy>& enemies, std::vector<particle_container>& particle_emitters, player& player, light_info& lightInfo)
{
	const auto gravity = -4.0f;
	const auto frame_time = 0.016f;
	const auto cutoff_for_air_frames = 2;
	
	const auto run_accel = 8.0f;
	const auto max_x_vel = 10.0f;
	const auto max_y_vel = 26.0f;
	const auto air_friction = 0.1f;

	player.acceleration.x = 0;
	player.acceleration.y = gravity;

	if (abs(player.velocity.x) > max_x_vel){
		player.velocity.x =  sign_of(player.velocity.x) * max_x_vel;
	}

	if (abs(player.velocity.y) > max_y_vel) {
		player.velocity.y = sign_of(player.velocity.y) * max_y_vel;
	}

	if (player.velocity.y < gravity * cutoff_for_air_frames * frame_time) {
		player.in_air = true;
	}

	for (const auto& plat : platforms) {
		auto result = plat.collides_with(player);
		if (result.is_some()) {
			auto penetration = result.unwrap().penetration;
			
			// translate along the axis with the minimium penetration
			if (abs(penetration.x) - abs(penetration.y) < 0.0f) {
				player.position.x += penetration.x;
				player.velocity.x = 0;
			} else {
				player.position.y += penetration.y;
				player.velocity.y = 0;
				// penetration is from the top, that means we're landing
				if (penetration.y > 0.0f) {
					player.in_air = false;
				}
			}

		}
	}

	for (auto& enm : enemies) {
		auto result = enm.collides_with(player);

		// on collision with enemy/hazard, respawn player
		if (result.is_some()) {
			player.position.x = 0;
			player.position.y = 0;
			player.velocity.x = 0;
			player.velocity.y = 0;
		}

		enm.update(dt);
	}

	for (auto& particle_em : particle_emitters) {
		particle_em.update(dt);
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000 && !player.in_air) {
		player.velocity.y = 4.0f;
		player.in_air = true;
	}
	else if (GetAsyncKeyState('S') & 0x8000) {
	}

	if (GetAsyncKeyState('A') & 0x8000 ) {
		player.acceleration.x = -run_accel * (player.in_air ? air_friction : 1.0f);
	}
	else if (GetAsyncKeyState('D') & 0x8000) {
		player.acceleration.x = run_accel * (player.in_air ? air_friction : 1.0f);
	}

	if (GetAsyncKeyState('F') & 0x8000) {
		particle_emitters[0].dt = 0;
	}

	if (GetAsyncKeyState('G') & 0x8000) {
		particle_emitters[1].dt = 0;
	}

	if (GetAsyncKeyState('H') & 0x8000) {
		particle_emitters[2].dt = 0;
	}

	// if player falls off the map, respawn
	if (player.position.y < -10) {
		player.position.x = 0;
		player.position.y = 0;
		player.velocity.x = 0;
		player.velocity.y = 0;
	}

	player.update(dt);
	
	const auto fluff = 8.0f;
	camera.position.x += (player.position.x - camera.position.x) * fluff * dt;
	camera.position.y += (player.position.y - (camera.position.y - 3)) * fluff * dt;

	DirectX::XMFLOAT4X4 shadow_view;
	DirectX::XMMATRIX sView = DirectX::XMMatrixLookToLH(
		DirectX::XMVectorSet(camera.position.x, 20, -20, 0),	// eye position
		DirectX::XMVectorSet(0.0f, -1, 1, 0),	// eye direction
		DirectX::XMVectorSet(0, 1, 0, 0));		// up direction, pos y, no touch
	XMStoreFloat4x4(&shadow_view, XMMatrixTranspose(sView));

	lightInfo.view = shadow_view;

	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE)) {
		done = true;
	}
}

void draw(dx_info& render_target, material& basic, material& particle_mat, 
	const std::vector<directional_light>& lights, const std::vector<platform>& platforms, const std::vector<enemy>& enemies, const std::vector<particle_container>& particle_emitters,
	const player& player, const camera& camera, sky_entity& sky, shadow_map& shadows , const light_info lightInfo, ID3D11BlendState& blend_state_transparent, ID3D11DepthStencilState& particle_depth_state)
{
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	/*
	draw shadow map here
	*/
	shadows.activate(render_target, lightInfo); // Sets up shadow map

	// actually render everything
	for (const auto& model : platforms) {
		model.draw_with_activated_shader(*render_target.device_context, shadows.shader);
	}

	for (const auto& model : enemies) {
		model.draw_with_activated_shader(*render_target.device_context, shadows.shader);
	}

	player.draw_with_activated_shader(*render_target.device_context, shadows.shader);

	shadows.deactivate(render_target);
	/*
	end draw shadow map	
	*/

	render_target.device_context->ClearRenderTargetView(render_target.render_target_view, color);
	render_target.device_context->ClearDepthStencilView(render_target.depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	basic.pixel.set_data("light", lights[0]);
	//basic.pixel.set_data("other_light", lights[1]);

	basic.pixel.set_shader_resource_view("shadowMap", shadows.shadow.resource_view);
	basic.pixel.set_sampler_state("shadowSampler", shadows.shadow.sampler_state);

	for (const auto& model : platforms) {
		model.draw(*render_target.device_context, camera, shadows);
	}

	for (const auto& model : enemies) {
		model.draw(*render_target.device_context, camera, shadows);
	}

	player.draw(*render_target.device_context, camera, shadows);
	


	// donw drawing solid things, unbind shadow map
	basic.pixel.set_shader_resource_view("shadowMap", 0);

	// done drawing solid stuff, draw skybox
	sky.draw(*render_target.device_context, camera);

	// Draw transparent

	// Turn on the state
	float factors[4] = { 1, 1, 1, 1 };
	render_target.device_context->OMSetBlendState(
		&blend_state_transparent,
		factors,
		0xFFFFFFFF);
	render_target.device_context->OMSetDepthStencilState(&particle_depth_state, 0);

	for (const auto& particle_em : particle_emitters) {
		if (particle_em.dt < particle_em.duration) {
			particle_em.draw(*render_target.device_context, camera);
		}

	}

	// Reset blend state for next frame
	render_target.device_context->OMSetBlendState(0, factors, 0xFFFFFFFF);
	render_target.device_context->OMSetDepthStencilState(0, 0);

	HR(render_target.swap_chain->Present(0, 0));
}
LRESULT CALLBACK WindowProc(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

int WINAPI WinMain(HINSTANCE app_instance, HINSTANCE hPrevInstance,	LPSTR command_line,	int command_show)
{

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	register_window_class(L"game", WindowProc, app_instance);
	auto window = create_window(L"game", L"test", app_instance, command_show);
	window.set_size(1280, 720);

	camera camera;
	camera.position = DirectX::XMFLOAT3(0, -2, -7);
	camera.rotation = DirectX::XMFLOAT2(3.14f / 8.0f, 0);

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//   the window resizes (which is already happening in OnResize() below)
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		0.45f * 3.1415926535f,		// Field of View Angle
		window.dx.aspect_ratio,				// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	DirectX::XMStoreFloat4x4(&camera.projection, DirectX::XMMatrixTranspose(P)); // Transpose for HLSL!

	MSG message;
	bool done = false;

	std::vector<directional_light> lights;

	lights.push_back(directional_light());
	lights.back().ambient_color = DirectX::XMFLOAT4(0.1f, 0.1f, 0.2f, 1.0f);
	lights.back().diffuse_color = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	lights.back().direction = DirectX::XMFLOAT3(0.0f, -1.0f, 1.0f);

	lights.push_back(directional_light());
	lights.back().ambient_color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lights.back().diffuse_color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	lights.back().direction = DirectX::XMFLOAT3(0.0f, -1.0f, 1.0f);

	// light matrices for shadow map - using main directional light
	DirectX::XMFLOAT4X4 shadow_view;
	DirectX::XMMATRIX sView = DirectX::XMMatrixLookToLH(
		DirectX::XMVectorSet(0, 20, -20, 0),	// eye position
		DirectX::XMVectorSet(0.0f, -1, 1, 0),	// eye direction
		DirectX::XMVectorSet(0, 1, 0, 0));		// up direction, pos y, no touch
	XMStoreFloat4x4(&shadow_view, XMMatrixTranspose(sView));
	DirectX::XMFLOAT4X4 shadow_proj;
	DirectX::XMMATRIX sProj = DirectX::XMMatrixOrthographicLH(
		50.0f,		// width in world units
		30.0f,		// height in wolrd units
		0.1f,		// near plane dist
		100.0f);	// far plane dist
	XMStoreFloat4x4(&shadow_proj, XMMatrixTranspose(sProj));

	light_info lightInfo;
	lightInfo.view = shadow_view;
	lightInfo.projection = shadow_proj;

	auto basic_texture = load_texture_from_file(L"demo1.jpg", *window.dx.device).take();
	auto mask_texture = load_texture_from_file(L"mask_texture.png", *window.dx.device).take();
	//auto platform_texture = load_texture_from_file(L"platform_texture.png", *window.dx.device).take();
	auto brick_texture = load_texture_from_file(L"bricks.jpg", *window.dx.device).take();
	auto particle_texture = load_texture_from_file(L"particle.png", *window.dx.device).take();
	auto spike_texture = load_texture_from_file(L"spikes.jpg", *window.dx.device).take();
	
	auto sky_texture = load_skybox(L"SunnyCubeMap.dds", *window.dx.device).take(); // sky dds file texture
	
	std::vector<mesh> meshes;
	meshes.push_back(load_mesh_from_file(R"(OBJ Files\platform.obj)", *window.dx.device).take());
	meshes.push_back(load_mesh_from_file(R"(OBJ Files\mask.obj)", *window.dx.device).take());
	meshes.push_back(load_mesh_from_file(R"(OBJ Files\spikes_v2.obj)", *window.dx.device).take());

	// shaders
	auto basic_pixel_shader = load_pixel_shader(L"PixelShader.cso", window.dx).take();
	auto basic_vertex_shader = load_vertex_shader(L"VertexShader.cso", window.dx).take();
	auto sky_pixel_shader = load_pixel_shader(L"SkyPS.cso", window.dx).take();
	auto sky_vertex_shader = load_vertex_shader(L"SkyVS.cso", window.dx).take();

	auto particle_pixel_shader = load_pixel_shader(L"ParticlePixelShader.cso", window.dx).take();
	auto particle_vertex_shader = load_vertex_shader(L"ParticleVertexShader.cso", window.dx).take();
	auto particle_geometry_shader = load_geometry_shader(L"ParticleGeometryShader.cso", window.dx).take();

	auto shadow_shader = load_vertex_shader(L"ShadowVertexShader.cso", window.dx).take();
	auto shadow_map = make_shadow_map(window.dx, shadow_shader, 1024);
 
	auto basic_material = make_material(basic_vertex_shader, basic_pixel_shader);
	auto sky_material = make_material(sky_vertex_shader, sky_pixel_shader);
	auto particle_material = make_material(particle_vertex_shader, particle_pixel_shader, particle_geometry_shader);

	auto sky = make_sky_entity(meshes[0], sky_material, sky_texture);

	std::vector<platform> platforms;
	platforms.push_back(make_platform(make_entity(meshes[0], basic_material, brick_texture), 4.0f, 0.6f));
	platforms.back().position.y = -3.25;
	platforms.back().position.x = 13.0;
	//platforms.back().scale.x = 4;
	//platforms.back().scale.y = 0.5;
	platforms.push_back(make_platform(make_entity(meshes[0], basic_material, brick_texture), 4.0f, 0.6f));
	platforms.back().position.y = -2.75;
	//platforms.back().scale.x = 10;
	//platforms.back().scale.y = 0.5;
	platforms.push_back(make_platform(make_entity(meshes[0], basic_material, brick_texture), 4.0f, 0.6f));
	platforms.back().position.y = -2.0;
	platforms.back().position.x = 8.0;
	//platforms.back().scale.x = 4.0;
	//platforms.back().scale.y = 0.5;

	// temp floor
	platforms.push_back(make_platform(make_entity(meshes[0], basic_material, basic_texture), 4.0f, 0.5f));
	platforms.back().position.y = -10;
	platforms.back().scale.x = 100;
	platforms.back().scale.z = 100;

	std::vector<enemy> enemies;
	enemies.push_back(make_spike(make_enemy(make_entity(meshes[2], basic_material, spike_texture))));
	enemies.back().position.y = -1.2;
	enemies.back().position.x = 9.0;

	std::vector<particle_container> particle_emitters;

	auto particle_emitter = make_particle(
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), // Transformations
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),		// Emit position
		100,										// Particle Amount
		2.0f,										// Duration
		1.0f,										// Start Speed
		0.0f,										// End Speed
		5.0f,										// Start Size
		0.0f,										// End Size
		6.29f,										// How much will each particle rotate
		true,										// Should start rotations be different
		6.29f,										// Max angle	(Using zero makes a circle using 2PI Makes a sphere)
		DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	// Start Color
		DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 0.0f),	// End Color
		0,											// Whether to use a sphere or half sphere (Should only be 0 or 1)
		particle_material, particle_texture, *window.dx.device);
	
	particle_emitters.push_back(particle_emitter);

	auto particle_emitter2 = make_particle(
		DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 90.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), // Transformations
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),		// Emit position
		100,										// Particle Amount
		2.0f,										// Duration
		0.01f,										// Start Speed
		1.0f,										// End Speed
		3.0f,										// Start Size
		0.0f,										// End Size
		6.29f,										// How much will each particle rotate
		true,										// Should start rotations be different
		0.0f,										// Max angle	(Using zero makes a circle using 2PI Makes a sphere)
		DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),	// Start Color
		DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),	// End Color
		0,											// Whether to use a sphere or half sphere (Should only be 0 or 1)
		particle_material, particle_texture, *window.dx.device);

	particle_emitters.push_back(particle_emitter2);

	auto particle_emitter3 = make_particle(
		DirectX::XMFLOAT3(6.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), // Transformations
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),		// Emit position
		100,										// Particle Amount
		3.0f,										// Duration
		0.1f,										// Start Speed
		0.8f,										// End Speed
		0.0f,										// Start Size
		10.0f,										// End Size
		3.14f,										// How much will each particle rotate
		true,										// Should start rotations be different
		6.29f,										// Max angle	(Using zero makes a circle using 2PI Makes a sphere)
		DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	// Start Color
		DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),	// End Color
		1,											// Whether to use a sphere or half sphere (Should only be 0 or 1)
		particle_material, particle_texture, *window.dx.device);


	particle_emitters.push_back(particle_emitter3);

	// Blend state to go along with the particles
	D3D11_BLEND_DESC blend_desc;
	ZeroMemory(&blend_desc, sizeof(blend_desc));

	blend_desc.AlphaToCoverageEnable = false;
	blend_desc.IndependentBlendEnable = false;
	blend_desc.RenderTarget[0].BlendEnable = true;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ID3D11BlendState* blend_state_transparent;
	window.dx.device->CreateBlendState(&blend_desc, &blend_state_transparent);

	// Depth stencil for particles
	D3D11_DEPTH_STENCIL_DESC depth_desc;
	ZeroMemory(&depth_desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depth_desc.DepthEnable = true;
	depth_desc.DepthFunc = D3D11_COMPARISON_LESS;
	depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	ID3D11DepthStencilState* particle_depth_state;
	window.dx.device->CreateDepthStencilState(&depth_desc, &particle_depth_state);


	auto player = make_player(make_entity(meshes[1], basic_material, brick_texture));

	uint64_t performance_frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&performance_frequency);
	auto performance_counter_seconds = 1.0 / (double)performance_frequency;

	uint64_t current_time;
	uint64_t previous_time;
	QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
	previous_time = current_time;


	while (!done) {
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
			if (message.message == WM_QUIT) {
				done = true;
			}
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
		auto dt = (float)((current_time - previous_time) * performance_counter_seconds);
		if (dt >= 0.016f) {
			previous_time = current_time;
			update(dt, done, camera, platforms, enemies, particle_emitters, player, lightInfo);
			draw(window.dx, basic_material, particle_material, 
				lights, platforms, enemies, particle_emitters, 
				player, camera, sky, 
				shadow_map , lightInfo, *blend_state_transparent, *particle_depth_state);
			// read all of the messages in the queue
		}
	}
	return WM_QUIT;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		};
	}

	// Handles any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}