#include "StateMachine.h"


StateMachine::StateMachine( ID3D11DeviceContext* ctx, ID3D11Device* device, texture& tex)
	:textureValue(tex)
{
	//sb = new DirectX::SpriteBatch(ctx);
	spriteBatch.reset(new DirectX::SpriteBatch(ctx));
	//spriteFont.reset(new DirectX::SpriteFont(device, L"myfile.spritefont"));
	keyDown = false;
	currentState = 0;
	frameCD = 0;
}


StateMachine::~StateMachine()
{
}

void StateMachine::Update() {
	if (GetAsyncKeyState('P') & 0x8000 && !keyDown && frameCD <= 0) {
		if (currentState == 1)
			currentState = 2;
		else
			currentState = 1;

		keyDown = true;
		frameCD = 25;
	}
	else if (frameCD <= 0) {
		keyDown = false;
	}

	frameCD--;
}

void StateMachine::DrawUI() {
	if (currentState == 2) {
		spriteBatch->Begin();
		spriteBatch->Draw(textureValue.resource_view, DirectX::SimpleMath::Vector2(550.0f, 325.0f));
		spriteBatch->End();
	}
}