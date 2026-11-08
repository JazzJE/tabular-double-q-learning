#include "tabular_q.h"
#include "policy_configs.h"
#include <random>

// Equality check for state-action pairs for hashing
bool StateActionPair::operator==(const StateActionPair& other) const
{
    if (state != other.state)
        return false;

    if (action != other.action)
        return false;

    return true;
}

// Simple polynomial hashing
size_t ValueFunction::StateActionPairHasher::operator()(const StateActionPair& sa) const
{
    size_t running_sum = 0;
    running_sum += static_cast<int>(sa.action);
    for (size_t i = 1; i <= GameConfigs::POSITION_DIMENSIONS; ++i)
        running_sum += sa.state[i - 1] * pow(POWER_PRIME, i);

    return std::hash<size_t>()(running_sum % MOD_PRIME);
}

void ValueFunction::update_state_action_pair(const StateActionPair& sa, double new_value)
{ 
    if (sa.state[0] == GameConfigs::TERMINAL_X_STATE && sa.state[1] == GameConfigs::TERMINAL_Y_STATE)
        return;
    value_function[sa] = new_value; 
}

// Check if the state-action pair exists inside of the current value function, and if not, then just return a random double value and update the state of the pair with this value
double ValueFunction::eval_state_action_pair(const StateActionPair& sa)
{
    if (sa.state[0] == GameConfigs::TERMINAL_X_STATE && sa.state[1] == GameConfigs::TERMINAL_Y_STATE)
        return 0;

    auto it = value_function.find(sa);
    if (it != value_function.end())
        return value_function[sa];

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> value_dis(ValueFunction::DEFAULT_RANDOM_MIN_VALUE, ValueFunction::DEFAULT_RANDOM_MAX_VALUE);
    double random_value = value_dis(gen);

    update_state_action_pair(sa, random_value);
    return random_value;
}

// Find the best possible action for the given state with epsilon-greedy mindset
PolicyConfigs::Action TabularQ::epsilon_select_best_action_combined(ValueFunction& vf1, ValueFunction& vf2, 
    const std::array<size_t, GameConfigs::POSITION_DIMENSIONS>& state)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> greedy_dis(0, 1);
    static std::uniform_int_distribution<int> action_dis(0, static_cast<size_t>(PolicyConfigs::Action::ACTION_COUNT) - 1);

    // If epsilon hits, then random action
    if (greedy_dis(gen) < PolicyConfigs::EPSILON)
        return PolicyConfigs::possible_actions[static_cast<size_t>(action_dis(gen))];
    // Else, choose action based on max sum of values
    else
    {
        size_t num_actions = static_cast<size_t>(PolicyConfigs::Action::ACTION_COUNT);
        // Select a random action to randomly break ties
        StateActionPair best_pair = StateActionPair(state, PolicyConfigs::possible_actions[static_cast<size_t>(action_dis(gen))]);
        for (size_t i = 0; i < num_actions; ++i)
        {
            StateActionPair curr_pair = StateActionPair(state, PolicyConfigs::possible_actions[i]);

            if (vf1.eval_state_action_pair(curr_pair) + vf2.eval_state_action_pair(curr_pair) >
                vf1.eval_state_action_pair(best_pair) + vf2.eval_state_action_pair(best_pair))
                best_pair = std::move(curr_pair);
        }

        return best_pair.action;
    }
}

// Given a value function, iterate over all possible actions and try to find the value of the 
PolicyConfigs::Action TabularQ::select_best_action(ValueFunction& value_function, const std::array<size_t, GameConfigs::POSITION_DIMENSIONS>& state)
{

    size_t num_actions = static_cast<size_t>(PolicyConfigs::Action::ACTION_COUNT);

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> action_dis(0, static_cast<size_t>(PolicyConfigs::Action::ACTION_COUNT) - 1);

    // Select a random action to break ties
    StateActionPair best_pair = StateActionPair(state, PolicyConfigs::possible_actions[static_cast<size_t>(action_dis(gen))]);
    for (size_t i = 0; i < num_actions; ++i)
    {
        StateActionPair curr_pair = StateActionPair(state, PolicyConfigs::possible_actions[i]);
        if (value_function.eval_state_action_pair(curr_pair) > value_function.eval_state_action_pair(best_pair))
            best_pair = std::move(curr_pair);
    }

    return best_pair.action;
}