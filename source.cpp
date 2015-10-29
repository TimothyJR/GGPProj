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

void update(float dt, bool& done, camera& camera, std::vector<platform>& platforms, std::vector<particle_container>& particle_emitters, player& player)
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


	player.update(dt);
	
	const auto fluff = 8.0f;
	camera.position.x += (player.position.x - camera.position.x) * fluff * dt;
	camera.position.y += (player.position.y - (camera.position.y - 3)) * fluff * dt;

	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE)) {
		done = true;
	}
}

void draw(dx_info& render_target, material& basic, material& particle_mat, const std::vector<directional_light>& lights, const std::vector<platform>& platforms, const std::vector<particle_container>& particle_emitters, const player& player, const camera& camera, sky_entity& sky)
{
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	render_target.device_context->ClearRenderTargetView(render_target.render_target_view, color);
	render_target.device_context->ClearDepthStencilView(render_target.depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	basic.pixel.set_data("light", lights[0]);
	basic.pixel.set_data("other_light", lights[1]);

	for (const auto& model : platforms) {
		model.draw(*render_target.device_context, camera);
	}
	player.draw(*render_target.device_context, camera);
	
	for (const auto& particle_em : particle_emitters) {
		particle_em.draw(*render_target.device_context, camera);
	}

	// done drawing solid stuff, draw skybox
	sky.draw(*render_target.device_context, camera);


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
	lights.back().ambient_color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lights.back().diffuse_color = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	lights.back().direction = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);

	lights.push_back(directional_light());
	lights.back().ambient_color = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	lights.back().diffuse_color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	lights.back().direction = DirectX::XMFLOAT3(0.0f, -1.0f, 1.0f);

	auto basic_texture = load_texture_from_file(L"demo1.jpg", *window.dx.device).take();
	auto sky_texture = load_skybox(L"SunnyCubeMap.dds", *window.dx.device).take(); // sky dds file texture

	std::vector<mesh> meshes;
	meshes.push_back(load_mesh_from_file(R"(OBJ Files\cube.obj)", *window.dx.device).take());

	// shaders
	auto basic_pixel_shader = load_pixel_shader(L"PixelShader.cso", window.dx).take();
	auto basic_vertex_shader = load_vertex_shader(L"VertexShader.cso", window.dx).take();
	auto sky_pixel_shader = load_pixel_shader(L"SkyPS.cso", window.dx).take();
	auto sky_vertex_shader = load_vertex_shader(L"SkyVS.cso", window.dx).take();

	auto particle_pixel_shader = load_pixel_shader(L"ParticlePixelShader.cso", window.dx).take();
	auto particle_vertex_shader = load_vertex_shader(L"ParticleVertexShader.cso", window.dx).take();
	auto particle_geometry_shader = load_geometry_shader(L"ParticleGeometryShader.cso", window.dx).take();

	auto basic_material = make_material(basic_vertex_shader, basic_pixel_shader);
	auto sky_material = make_material(sky_vertex_shader, sky_pixel_shader);
	auto particle_material = make_material(particle_vertex_shader, particle_pixel_shader, particle_geometry_shader);

	auto sky = make_sky_entity(meshes[0], sky_material, sky_texture);

	std::vector<platform> platforms;
	platforms.push_back(make_platform(make_entity(meshes[0], basic_material, basic_texture), 4000.0f, 0.25f));
	platforms.back().position.y = -2.75;
	platforms.back().scale.x = 4000;
	platforms.back().scale.y = 0.25;
	platforms.push_back(make_platform(make_entity(meshes[0], basic_material, basic_texture), 4.0f, 0.25f));
	platforms.back().position.y = -2.0;
	platforms.back().position.x = 6.0f;
	platforms.back().scale.x = 4.0f;
	platforms.back().scale.y = 0.25;

	std::vector<particle_container> particle_emitters;

	auto particle_emitter = make_particle(
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		15,
		10.0f,
		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), particle_material, *window.dx.device);
	
	particle_emitters.push_back(particle_emitter);

	auto player = make_player(make_entity(meshes[0], basic_material, basic_texture));

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
			update(dt, done, camera, platforms, particle_emitters, player);
			draw(window.dx, basic_material, particle_material, lights, platforms, particle_emitters, player, camera, sky);
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