#pragma once
#include <unordered_map>
#include <array>
#include <cmath>
#include "policy_configs.h"
#include "game.h"

struct StateActionPair
{
	std::array<size_t, GameConfigs::POSITION_DIMENSIONS> state;
	PolicyConfigs::Action action;

	StateActionPair(std::array<size_t, GameConfigs::POSITION_DIMENSIONS> input_state, 
		PolicyConfigs::Action input_action) : state(input_state), action(input_action) {}

	bool operator==(const StateActionPair& other) const;
};

class ValueFunction
{
	private:	
		struct StateActionPairHasher
		{
			static constexpr size_t MOD_PRIME = 1000000009;
			static constexpr size_t POWER_PRIME = 31;

			size_t operator()(const StateActionPair& sa) const;
		};

		std::unordered_map<StateActionPair, double, StateActionPairHasher> value_function;

		inline static double DEFAULT_RANDOM_MIN_VALUE = 0.0;
		inline static double DEFAULT_RANDOM_MAX_VALUE = 30.0;

	public: 

		void update_state_action_pair(const StateActionPair& sa, double new_value);
		double eval_state_action_pair(const StateActionPair& sa);
};

namespace TabularQ
{
	// Select a best action in the current state from summing the values from both functions
	PolicyConfigs::Action epsilon_select_best_action_combined(ValueFunction& vf1, ValueFunction& vf2,
		const std::array<size_t, GameConfigs::POSITION_DIMENSIONS>& state);
	PolicyConfigs::Action select_best_action(ValueFunction& value_function, 
		const std::array<size_t, GameConfigs::POSITION_DIMENSIONS>& state); // Select a best action in the current state

};