#pragma once

namespace PolicyConfigs
{
	enum Action { UP, DOWN, LEFT, RIGHT };

	// Evaluation hyperparameters
	constexpr double EPSILON = 0.1;
	constexpr double GAMMA = 0.9;

	// Rewards for "good" actions
	constexpr double GAS_REWARD = 50;
	constexpr double TERMINAL_REWARD = 200;
	constexpr double KIT_REWARD = 80;

	// Rewards for "bad" actions
	constexpr double TRAVEL_PENALTY = -1;
	constexpr double DAMAGED_PENALTY = -20;
	constexpr double DEATH_PENALTY = -100;
	constexpr double BOUNDARY_EXIT_PENALTY = -5;
};
