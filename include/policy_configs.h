#pragma once

namespace PolicyConfigs
{
	enum Action { UP, DOWN, LEFT, RIGHT };
	constexpr Action possible_actions[] = { UP, DOWN, LEFT, RIGHT };

	static inline size_t epochs = 0;
	static void increment_epochs(size_t n = 1) { epochs += n; }

	// Evaluation hyperparameters
	constexpr double EPSILON = 0.1;
	constexpr double DISCOUNT_RATE = 0.9;

	// Rewards for "good" actions
	constexpr double GAS_REWARD = 50;
	constexpr double TERMINAL_REWARD = 200;

	// Rewards for "bad" actions
	constexpr double TRAVEL_PENALTY = -1;
	constexpr double TRAP_PENALTY = -20;
	constexpr double BOUNDARY_EXIT_PENALTY = -5;
};
