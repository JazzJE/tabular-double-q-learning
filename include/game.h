#pragma once
#include <unordered_set>
#include <array>
#include <vector>

class Sprite;

namespace GameConfigs
{
	// Screen dimensions and other GUI settings
	constexpr size_t SCREEN_HEIGHT = 1600;
	constexpr size_t GUI_WIDTH = 200;
	constexpr size_t SCREEN_WIDTH = SCREEN_HEIGHT + GUI_WIDTH;

	// Tile configurations
	constexpr size_t TILE_SIZE = SCREEN_HEIGHT / 100;
	constexpr size_t TILE_COLOR = 0xFFFFFFFF;
	constexpr size_t TILE_BORDER_COLOR = 0x000000FF;

	// Bot and sprite configurations
	constexpr size_t SPRITE_RADIUS = TILE_SIZE / 2;
	constexpr size_t DEFAULT_BOT_COLOR = 0x000000FF;
	constexpr size_t DEFAULT_TRAP_COLOR = 0xFF0000FF;
	constexpr size_t DEFAULT_GAS_COLOR = 0x00FF00FF;
	constexpr size_t DEFAULT_VISITED_STATE_COLOR = 0x0000FF33;
	// Cosmetic effect for centering sprites in the middle of their tile
	constexpr double SPRITE_Y_OFFSET = -0.05;
	constexpr double SPRITE_X_OFFSET = -0.005;

	// Number of traps and gas sprites
	constexpr size_t NUM_GAS = 200;
	constexpr size_t NUM_SPIKES = 300;

	// Game config
	constexpr size_t POSITION_DIMENSIONS = 2;
	constexpr size_t BOT_START_X_STATE = 0;
	constexpr size_t BOT_START_Y_STATE = 0;
	constexpr size_t GAME_MAX_X_STATE = (SCREEN_WIDTH - GUI_WIDTH) / TILE_SIZE - 1;
	constexpr size_t GAME_MAX_Y_STATE = SCREEN_HEIGHT / TILE_SIZE - 1;
	constexpr size_t TERMINAL_X_STATE = GAME_MAX_X_STATE;
	constexpr size_t TERMINAL_Y_STATE = GAME_MAX_Y_STATE;
};

namespace GameProcesses
{
	void run();
	void draw_map();
	void draw_visited_states(const Sprite& bot, bool reset_states = false);

	// Generate trap and gas sprites
	struct ArrayPositionHasher
	{ const size_t MOD_PRIME = pow(10, 9) + 9; const size_t POWER_PRIME = 31;
		size_t operator()(const std::array<size_t, GameConfigs::POSITION_DIMENSIONS>& array_position) const; };
	std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, ArrayPositionHasher> 
		generate_unique_positions(size_t num_positions, size_t min_range = 0, size_t max_range = GameConfigs::GAME_MAX_Y_STATE);
	// This method will directly pop elements out of the set of arrays (unique_positions)
	template<typename T>
	std::vector<T> generate_sprites(std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, ArrayPositionHasher>& unique_positions, 
		size_t num_sprites = 0);

	// Collisions
	template<typename T, typename S> 
	bool check_collision_between_sprites(const T& first, const S& second);
	template<typename T, typename S> 
	bool check_collision_between_sprite_and_group(const T& sprite, const std::vector<S>& group);

	// Set the position of sprites visually based on what state they are in
	template<typename T>
	void draw_sprites(const T& last);
	template<typename T, typename... Args>
	void draw_sprites(const T& first, const Args&... rest);
	template<typename T>
	void draw_group(const std::vector<T>& group);
};