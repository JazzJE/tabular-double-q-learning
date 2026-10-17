#pragma once
#include "raylib.h"
#include "game.h"

class Sprite
{
	protected:
		Color sprite_color;
		Vector2 position;

	public:
		
		Sprite(
			Vector2 starting_position = {0.0, 0.0}, 
			Color starting_color = GetColor(GameConfigs::DEFAULT_BOT_COLOR)
		)
			: position(starting_position), sprite_color(starting_color) {}

		void set_position(const Vector2& new_position) { position = new_position; }
		Vector2 get_position() const { return position; }

		void set_color(const Color& new_color) { sprite_color = new_color; }
		Color get_color() const { return sprite_color; }
};