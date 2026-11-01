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
			const size_t MOD_PRIME = pow(10, 9) + 9;
			const size_t POWER_PRIME = 31;

			size_t operator()(const StateActionPair& sa) const;
		};

		std::unordered_map<StateActionPair, double, StateActionPairHasher> value_function;

		inline static double DEFAULT_RANDOM_MIN_VALUE = 0.0;
		inline static double DEFAULT_RANDOM_MAX_VALUE = 100.0;

	public: 

		ValueFunction(); // Fix the terminal state to always have a value of 0 for every action
		void update_state_action_pair(const StateActionPair& sa, double new_value);
		double eval_state_action_pair(const StateActionPair& sa);
};

class TabularQ
{
	private:
		ValueFunction vf1;
		ValueFunction vf2;

	public:
		// Select a best action in the current state from 
		PolicyConfigs::Action epsilon_select_action_combined();


		PolicyConfigs::Action epsilon_select_action_first();
		PolicyConfigs::Action epsilon_select_action_second();

};