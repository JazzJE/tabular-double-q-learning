#pragma once
#include "sprite.h"

class TrapSprite : public Sprite
{
	private:
		size_t damage;

	public:
		TrapSprite(
			Vector2 starting_position = { 0.0, 0.0 },
			Color starting_color = GetColor(GameConfigs::DEFAULT_TRAP_COLOR),
			size_t starting_damage = 1
		) : Sprite(starting_position, starting_color), damage(starting_damage) {}
};