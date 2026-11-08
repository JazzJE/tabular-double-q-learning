#include <iostream>
#include <array>
#include "policy_configs.h"
#include "tabular_q.h"
#include "game.h"

void test_value_function_hashing();
void test_generate_unique_positions();

int main()
{
	test_value_function_hashing();
	return 0;
}

// Test if the value function can take in StateActionPair objects and reliably return double values from them
void test_value_function_hashing()
{
	std::array<size_t, GameConfigs::POSITION_DIMENSIONS> state_one = { 5, 8 };
	std::array<size_t, GameConfigs::POSITION_DIMENSIONS> state_two = { 3, 2 };

	StateActionPair sa_one(state_one, PolicyConfigs::Action::UP);
	StateActionPair sa_two(state_two, PolicyConfigs::Action::DOWN);

	ValueFunction val;

	// Test if we can access the value stored in the state if we assign it
	val.update_state_action_pair(sa_one, 500);
	std::cout << std::endl << "SA-pair object (sa_one, state_one, up): " << val.eval_state_action_pair(sa_one) << std::endl;

	std::cout << std::endl << "===================================" << std::endl;

	// Test if a different state action pair can access the value of the same state action pair
	StateActionPair sa_three(state_one, PolicyConfigs::Action::UP);
	std::cout << std::endl << "Different SA-pair object, but same state and action (sa_three, state_one, up): " << val.eval_state_action_pair(sa_three) << std::endl;

	std::cout << std::endl << "===================================" << std::endl;

	// Test if the state of the value function will be adapted if we first initialize it to a random value by just evaluating it 
	std::cout << std::endl << "Random SA-pair value (sa_two, state_two, down): " << val.eval_state_action_pair(sa_two) << std::endl;
	val.update_state_action_pair(sa_two, 1000);
	std::cout << std::endl << "Updated random SA-pair value to 1000 (sa_two, state_two, down): " << val.eval_state_action_pair(sa_two) << std::endl;

	std::cout << std::endl << "===================================" << std::endl;

	// Test if different actions but the same state will still return different values
	StateActionPair sa_four(state_two, PolicyConfigs::Action::RIGHT);
	val.update_state_action_pair(sa_four, 1500);
	std::cout << std::endl << "SA-pair value of state two, action down (sa_two, state_two, down): " << val.eval_state_action_pair(sa_two) << std::endl;
	std::cout << std::endl << "SA-pair value of state two, action right (sa_four, state_two, right): " << val.eval_state_action_pair(sa_four) << std::endl;

	std::cout << std::endl << "===================================" << std::endl;

	// Test if the select_best_action selects the action that maximizes the value given the current state
	ValueFunction vf;
	std::cout << std::endl << "Max value SA-pair (state_one): " << TabularQ::select_best_action(vf, state_one) << std::endl;
	for (size_t i = 0; i < static_cast<size_t>(PolicyConfigs::Action::ACTION_COUNT); ++i)
		std::cout << std::endl << "SA-pair value (state_one, action " 
			<< static_cast<size_t>(PolicyConfigs::possible_actions[i]) << "): " 
		<< vf.eval_state_action_pair(StateActionPair(state_one, PolicyConfigs::possible_actions[i]));
	
	std::cout << std::endl << "===================================" << std::endl;

	// Test if the select_best_action selects an appropriate action even if there are two maxes
	// Set the second action to the first action's value, who will be fixed to a max
	vf.update_state_action_pair(StateActionPair(state_one, PolicyConfigs::possible_actions[1]), 99999);
	vf.update_state_action_pair(StateActionPair(state_one, PolicyConfigs::possible_actions[1]),
		vf.eval_state_action_pair(StateActionPair(state_one, PolicyConfigs::possible_actions[0])));
	std::cout << std::endl << "Max value SA-pair (state_one): " << TabularQ::select_best_action(vf, state_one) << std::endl;
	for (size_t i = 0; i < static_cast<size_t>(PolicyConfigs::Action::ACTION_COUNT); ++i)
		std::cout << std::endl << "SA-pair value (state_one, action "
		<< static_cast<size_t>(PolicyConfigs::possible_actions[i]) << "): "
		<< vf.eval_state_action_pair(StateActionPair(state_one, PolicyConfigs::possible_actions[i]));
	std::cout << std::endl;
}

void test_generate_unique_positions()
{
	size_t num_unique_positions = 15;
	std::cout << std::endl << "Generating " << num_unique_positions << " unique positions...";
	std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, GameProcesses::ArrayPositionHasher> 
		unique_positions = GameProcesses::generate_unique_positions(num_unique_positions);

	size_t counter = 1;
	for (auto it = unique_positions.begin(); it != unique_positions.end(); ++it)
	{
		std::cout << std::endl << "Position " << counter << ": ";
		for (size_t ele : *it)
			std::cout << ele << " ";
		counter += 1;
	}

	std::cout << std::endl << "===================================" << std::endl;

	std::cout << std::endl << "Inserting beginning of the set into the set...";
	if (!unique_positions.insert(*unique_positions.begin()).second)
		std::cout << std::endl << "Failed to insert! (Good)" << std::endl;
	else
		std::cout << std::endl << "Succeeded to insert! (TERRIBLE)" << std::endl;

	std::cout << std::endl << "===================================" << std::endl;

	num_unique_positions = 0;
	std::cout << std::endl << "Generating " << num_unique_positions << " unique positions...";
	unique_positions = GameProcesses::generate_unique_positions(num_unique_positions);
	std::cout << std::endl << "Size of set: " << unique_positions.size() << std::endl;
}