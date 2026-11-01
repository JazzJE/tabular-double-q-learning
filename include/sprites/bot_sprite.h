#pragma once
#include "sprite.h"
#include "policy_configs.h"
#include "game.h"
#include "raylib.h"

class BotSprite : public Sprite
{
	private:
		Vector2 previous_position;
		PolicyConfigs::Action previous_action;

	public:
		BotSprite(
			Vector2 starting_position = {0.0, 0.0},
			Color starting_color = GetColor(GameConfigs::DEFAULT_BOT_COLOR), 
			size_t starting_lives = 3
		)
			: Sprite(starting_position, starting_color) {}

		void take_action(PolicyConfigs::Action a);

		Vector2 get_previous_position() { return previous_position; }
		PolicyConfigs::Action get_previous_action() { return previous_action; }
};