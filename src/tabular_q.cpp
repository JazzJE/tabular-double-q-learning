#include "tabular_q.h"
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
{ value_function[sa] = new_value; }

// Check if the state-action pair exists inside of the current value function, and if not, then just return a random double value and update the state of the pair with this value
double ValueFunction::eval_state_action_pair(const StateActionPair& sa)
{
    auto it = value_function.find(sa);
    if (it != value_function.end())
        return value_function[sa];

    double lower = ValueFunction::DEFAULT_RANDOM_MIN_VALUE, upper = ValueFunction::DEFAULT_RANDOM_MAX_VALUE;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(lower, upper);
    double random_value = dis(gen);

    update_state_action_pair(sa, random_value);
    return random_value;
}