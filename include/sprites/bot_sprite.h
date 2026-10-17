#pragma once
#include "sprite.h"
#include "policy_configs.h"
#include "game.h"
#include "raylib.h"

class BotSprite : public Sprite
{
	public:
		BotSprite(
			Vector2 starting_position = {0.0, 0.0},
			Color starting_color = GetColor(GameConfigs::DEFAULT_BOT_COLOR), 
			size_t starting_lives = 3
		)
			: Sprite(starting_position, starting_color) {}

		void take_action(PolicyConfigs::Action a);

};