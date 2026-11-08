#pragma once

namespace PolicyConfigs
{
	enum Action { UP, DOWN, LEFT, RIGHT, ACTION_COUNT };
	constexpr Action possible_actions[] = { UP, DOWN, LEFT, RIGHT };

	static inline size_t epochs = 0;
	static void increment_epochs(size_t n = 1) { epochs += n; }

	// Evaluation hyperparameters
	constexpr size_t MAX_STEPS = 10000;
	static inline size_t curr_steps = 0;
	static inline double EPSILON = 0.3;
	constexpr double DISCOUNT_RATE = 0.95;
	static inline double LEARNING_RATE = 0.6;

	// Rewards for "good" actions
	constexpr double ROCK_REWARD = 60;
	constexpr double TERMINAL_REWARD = 400;

	// Rewards for "bad" actions
	constexpr double TRAVEL_PENALTY = -0.5;
	constexpr double REVISIT_PENALTY = -5;
	constexpr double TRAP_PENALTY = -30;
	constexpr double BOUNDARY_EXIT_PENALTY = -30;
};
