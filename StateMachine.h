#pragma once
#include <memory>
#include "debug.h"
#include "SpriteBatch.h"
#include "Material.h"
#include "SimpleMath.h"
#include "dxi_ptr.h"
#include "DDSTextureLoader.h"

class state_machine
{
	//dxi_ptr<DirectX::SpriteBatch> sprite_batch;
	//dxi_ptr<DirectX::SpriteFont> spriteFont;
	std::unique_ptr<DirectX::SpriteBatch> sprite_batch;
	//std::unique_ptr<DirectX::SpriteFont> spriteFont;
	//states 0=menu, 1=running, 2=paused
	int current_state;
	//never changes state if key is held down, need to wait so much time before pausing again
	texture& texture_value;
	bool key_down;
	int frame_cd;

public:
	state_machine(ID3D11DeviceContext* ctx, ID3D11Device* device, texture& tex);
	~state_machine();
	void update();
	void draw_ui();
	int get_state() { return current_state; }
};