#pragma once
#include "sprite.h"

class RockSprite : public Sprite
{
	public:
		RockSprite(
			Vector2 starting_position = { 0.0, 0.0 },
			Color starting_color = GetColor(GameConfigs::DEFAULT_ROCK_COLOR)
		)
			: Sprite(starting_position, starting_color) {}
};