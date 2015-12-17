#include "StateMachine.h"


state_machine::state_machine( ID3D11DeviceContext* ctx, ID3D11Device* device, texture& tex)
	:texture_value(tex)
{
	//sb = new DirectX::SpriteBatch(ctx);
	sprite_batch.reset(new DirectX::SpriteBatch(ctx));
	//spriteFont.reset(new DirectX::SpriteFont(device, L"myfile.spritefont"));
	key_down = false;
	current_state = 0;
	frame_cd = 0;
}


state_machine::~state_machine()
{
}

void state_machine::update() {
	if (GetAsyncKeyState('P') & 0x8000 && !key_down && frame_cd <= 0) {
		if (current_state == 1)
			current_state = 2;
		else
			current_state = 1;

		key_down = true;
		frame_cd = 25;
	}
	else if (frame_cd <= 0) {
		key_down = false;
	}

	frame_cd--;
}

void state_machine::draw_ui() {
	if (current_state == 2) {
		sprite_batch->Begin();
		sprite_batch->Draw(texture_value.resource_view, DirectX::SimpleMath::Vector2(550.0f, 325.0f));
		sprite_batch->End();
	}
}