#pragma once
#include "sprite.h"

class GasSprite : public Sprite
{
	public:
		GasSprite(
			Vector2 starting_position = { 0.0, 0.0 },
			Color starting_color = GetColor(GameConfigs::DEFAULT_GAS_COLOR)
		)
			: Sprite(starting_position, starting_color) {}
};