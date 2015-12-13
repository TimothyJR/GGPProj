#pragma once
#include <memory>
#include "debug.h"
#include "SpriteBatch.h"
#include "Material.h"
#include "SimpleMath.h"
#include "dxi_ptr.h"
#include "DDSTextureLoader.h"

class StateMachine
{
	//dxi_ptr<DirectX::SpriteBatch> spriteBatch;
	//dxi_ptr<DirectX::SpriteFont> spriteFont;
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	//std::unique_ptr<DirectX::SpriteFont> spriteFont;
	//states 0=menu, 1=running, 2=paused
	int currentState;
	//never changes state if key is held down, need to wait so much time before pausing again
	texture& textureValue;
	bool keyDown;
	int frameCD;

public:
	StateMachine(ID3D11DeviceContext* ctx, ID3D11Device* device, texture& tex);
	~StateMachine();
	void Update();
	void DrawUI();
	int GetState() { return currentState; }
};