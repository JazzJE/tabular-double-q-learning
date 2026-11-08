#include <unordered_set>
#include <set>
#include <array>
#include <random>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits> 
#include <algorithm>
#include <cmath>  
#include <string>
#include <fstream>
#include "tabular_q.h"
#include "bot_sprite.h"
#include "rock_sprite.h"
#include "trap_sprite.h"
#include "game.h"
#include "raylib.h"

void GameProcesses::run()
{
    // Create a 2D array of all possible positions for the map
    InitWindow(GameConfigs::SCREEN_WIDTH, GameConfigs::SCREEN_HEIGHT, "Double Q-Learning Demonstration");
    SetTargetFPS(GameConfigs::MAX_FPS);

    Vector2 bot_start = { 0, 0 };
    BotSprite bot(bot_start);
    
    std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, GameProcesses::ArrayPositionHasher> 
        unique_positions = GameProcesses::generate_unique_positions(GameConfigs::NUM_ROCK + GameConfigs::NUM_TRAPS);

    // Action selector
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> val_update_dis(0, 1);

    // Create a vector of all the rock sprites
    std::vector<RockSprite> rock_group = generate_sprites<RockSprite>(unique_positions, GameConfigs::NUM_ROCK);
    // Create a vector of all the rock sprites removed from the current episode to be reappended later
    std::vector<RockSprite> removed_rock_group;

    // Create a vector of all the trap sprites
    std::vector<TrapSprite> trap_group = generate_sprites<TrapSprite>(unique_positions, GameConfigs::NUM_TRAPS);

    // Create two value functions
    ValueFunction vf1;
    ValueFunction vf2;

    bool reset_visited_states = false;
    bool visualize_learning = true;
    // Counters to keep track of how many times the bot exits the boundary or collides with a trap
    size_t trap_collision_count = 0, exit_boundary_count = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        double net_reward = 0;
        PolicyConfigs::curr_steps += 1;

        // If we've reached the terminal state or we've exceed the number of steps taken, reset the game and continue
        if (GameProcesses::reached_end(bot) || PolicyConfigs::curr_steps > PolicyConfigs::MAX_STEPS)
        {
            GameProcesses::reset_game(bot);
            GameProcesses::reappend_removed_group_sprites(rock_group, removed_rock_group);
            PolicyConfigs::increment_epochs();
            EndDrawing();
            continue;
        }
        

        Vector2 prev_position = bot.get_position();
        std::array<size_t, GameConfigs::POSITION_DIMENSIONS> prev_state_array = { prev_position.x, prev_position.y };
        PolicyConfigs::Action prev_epsilon_best_action = TabularQ::epsilon_select_best_action_combined(vf1, vf2, prev_state_array);
        StateActionPair prev_pair = StateActionPair(prev_state_array, prev_epsilon_best_action);
        bot.take_action(prev_epsilon_best_action);

        // Keybind options and processing
        if (IsKeyDown(KEY_UP) && GameConfigs::MAX_FPS < GameConfigs::SUP_FPS)
        {
            GameConfigs::MAX_FPS += 1;
            SetTargetFPS(GameConfigs::MAX_FPS);
        }
        if (IsKeyDown(KEY_DOWN) && GameConfigs::MAX_FPS > GameConfigs::INF_FPS)
        {
            GameConfigs::MAX_FPS -= 1;
            SetTargetFPS(GameConfigs::MAX_FPS);
        }

        if (IsKeyDown(KEY_W))
            PolicyConfigs::EPSILON = std::min(1.0, PolicyConfigs::EPSILON + 0.01);
        if (IsKeyDown(KEY_S))
            PolicyConfigs::EPSILON = std::max(0.0, PolicyConfigs::EPSILON - 0.01);
        if (IsKeyDown(KEY_E) && PolicyConfigs::LEARNING_RATE < 1)
            PolicyConfigs::LEARNING_RATE = std::min(1.0, PolicyConfigs::LEARNING_RATE + 0.01);
        if (IsKeyDown(KEY_D) && PolicyConfigs::LEARNING_RATE > 0)
            PolicyConfigs::LEARNING_RATE = std::max(0.0, PolicyConfigs::LEARNING_RATE - 0.01);

        if (GameProcesses::timer < GameConfigs::BUTTON_COOLDOWN)
            GameProcesses::timer += GetFrameTime();

        if (IsKeyDown(KEY_C) && GameProcesses::timer > GameConfigs::BUTTON_COOLDOWN)
        { reset_visited_states = !reset_visited_states; GameProcesses::timer = 0.0; }
        if (IsKeyDown(KEY_P) && GameProcesses::timer > GameConfigs::BUTTON_COOLDOWN)
        { visualize_learning = !visualize_learning; GameProcesses::timer = 0.0; }

        // Penalties and enforcing boundary checks
        net_reward += PolicyConfigs::TRAVEL_PENALTY;
        if (GameProcesses::check_escaped_boundary(bot))
        {
            GameConfigs::BOUNDARY_EXIT_COUNTER += 1;
            net_reward += PolicyConfigs::BOUNDARY_EXIT_PENALTY;
            GameProcesses::enforce_boundary(bot);
        }
        if (GameProcesses::check_collision_between_sprite_and_group(bot, trap_group))
        {
            GameConfigs::TRAP_COLLISION_COUNTER += 1;
            net_reward += PolicyConfigs::TRAP_PENALTY;
            bot.set_position(prev_position);
        }

        // Get the position only after enforcing boundary checks
        std::array<size_t, GameConfigs::POSITION_DIMENSIONS> curr_state_array = { bot.get_position().x, bot.get_position().y };
        GameProcesses::visited_states.insert(curr_state_array);

        if (GameProcesses::visited_states.find(curr_state_array) != GameProcesses::visited_states.end())
            net_reward += PolicyConfigs::REVISIT_PENALTY;

        // Rewards
        if (GameProcesses::check_collision_between_sprite_and_group(bot, rock_group))
        {
            net_reward += PolicyConfigs::ROCK_REWARD;
            GameProcesses::remove_sprite_at_state_from_group(curr_state_array, rock_group, removed_rock_group);
        }
        if (GameProcesses::reached_end(bot))
            net_reward += PolicyConfigs::TERMINAL_REWARD;

        // Get the previous value, the max value of the current state, and the new (q-learning updated) value of the state
        double prev_value, new_value, curr_max_value;
        // Update rule; half chance to update value function 1, half chance to update value function 2
        bool update_first_vf = (val_update_dis(gen) < 0.5) ? 1 : 0;
        if (update_first_vf)
        {
            prev_value = vf1.eval_state_action_pair(prev_pair);
            PolicyConfigs::Action curr_max_action = TabularQ::select_best_action(vf1, curr_state_array);
            StateActionPair curr_max_pair = StateActionPair(curr_state_array, curr_max_action);

            curr_max_value = vf2.eval_state_action_pair(curr_max_pair);
            new_value = prev_value + PolicyConfigs::LEARNING_RATE * (net_reward + PolicyConfigs::DISCOUNT_RATE * curr_max_value - prev_value);

            vf1.update_state_action_pair(prev_pair, new_value);
        }
        else
        {
            prev_value = vf2.eval_state_action_pair(prev_pair);
            PolicyConfigs::Action curr_max_action = TabularQ::select_best_action(vf2, curr_state_array);
            StateActionPair curr_max_pair = StateActionPair(curr_state_array, curr_max_action);

            curr_max_value = vf1.eval_state_action_pair(curr_max_pair);
            new_value = prev_value + PolicyConfigs::LEARNING_RATE * (net_reward + PolicyConfigs::DISCOUNT_RATE * curr_max_value - prev_value);

            vf2.update_state_action_pair(prev_pair, new_value);
        }

        // Drawing sprites to screen IFF the user wants to visualize the learning process, else just draw "training"
        if (visualize_learning)
        {
            GameProcesses::draw_visited_states(reset_visited_states);
            GameProcesses::draw_group(rock_group);
            GameProcesses::draw_group(trap_group);
            GameProcesses::draw_sprites(bot);
            GameProcesses::draw_map();
        }
        else
            GameProcesses::draw_training_screen();

        // Draw the menu on the right of the game
        GameProcesses::draw_stats_screen(bot, update_first_vf, prev_value, curr_max_value, new_value, net_reward, rock_group, trap_group);

        EndDrawing();
    }

    CloseWindow();
    return;
}

void GameProcesses::draw_map()
{
    // Draw the terminal state
    DrawRectangle(GameConfigs::TERMINAL_X_STATE * GameConfigs::TILE_SIZE,
        GameConfigs::TERMINAL_Y_STATE * GameConfigs::TILE_SIZE,
        GameConfigs::SPRITE_RADIUS * 2, GameConfigs::SPRITE_RADIUS * 2,
        GetColor(GameConfigs::DEFAULT_TERMINAL_STATE_COLOR));

    // Draw vertical lines
    for (size_t s = 0; s * GameConfigs::TILE_SIZE <= GameConfigs::SCREEN_WIDTH - GameConfigs::GUI_WIDTH; ++s)
        DrawLine(s * GameConfigs::TILE_SIZE, 0, s * GameConfigs::TILE_SIZE, GameConfigs::SCREEN_HEIGHT, 
            GetColor(GameConfigs::TILE_BORDER_COLOR));

    // Draw horizontal lines
    for (size_t s = 0; s * GameConfigs::TILE_SIZE <= GameConfigs::SCREEN_HEIGHT; ++s)
        DrawLine(0, s * GameConfigs::TILE_SIZE, GameConfigs::SCREEN_WIDTH - GameConfigs::GUI_WIDTH, s * GameConfigs::TILE_SIZE,
            GetColor(GameConfigs::TILE_BORDER_COLOR));
}

bool GameProcesses::reached_end(const Sprite& sprite)
{ return sprite.get_position().x == GameConfigs::TERMINAL_X_STATE && sprite.get_position().y == GameConfigs::TERMINAL_Y_STATE; }
void GameProcesses::reset_game(Sprite& sprite)
{
    Vector2 start_position;
    start_position.x = GameConfigs::BOT_START_X_STATE;
    start_position.y = GameConfigs::BOT_START_Y_STATE;

    bool reset_visited_states = true;
    GameProcesses::draw_visited_states(reset_visited_states);

    GameConfigs::TRAP_COLLISION_COUNTER = 0;
    GameConfigs::BOUNDARY_EXIT_COUNTER = 0;
    PolicyConfigs::curr_steps = 0;

    sprite.set_position(start_position);
}

// Fill each state visited with a circle
void GameProcesses::draw_visited_states(bool reset_visited_states)
{
    if (reset_visited_states)
        visited_states.clear();

    for (auto it = visited_states.begin(); it != visited_states.end(); ++it)
        DrawCircle((*it)[0] * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_X_OFFSET,
            (*it)[1] * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_Y_OFFSET,
            GameConfigs::SPRITE_RADIUS, GetColor(GameConfigs::DEFAULT_VISITED_STATE_COLOR));
}

// Draw the game menu to the right of the game
void GameProcesses::draw_stats_screen(const BotSprite& bot, bool update_first_vf, double prev_value, double curr_max_value, 
    double new_value, double net_reward, const std::vector<RockSprite>& rock_group, const std::vector<TrapSprite>& trap_group)
{
    // Position
    DrawText("BOT DESC.", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS, GameConfigs::DEFAULT_TITLE_FONT_SIZE, GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("X POS: %i", static_cast<size_t>(bot.get_position().x)), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 1.25, GameConfigs::DEFAULT_STATS_FONT_SIZE, GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("Y POS: %i", static_cast<size_t>(bot.get_position().y)), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 2.25, GameConfigs::DEFAULT_STATS_FONT_SIZE, 
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("TRAP COLLIDE COUNT: %i", GameConfigs::TRAP_COLLISION_COUNTER), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 3.25, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("BOUND EXIT COUNT: %i", GameConfigs::BOUNDARY_EXIT_COUNTER), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 4.25, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));

    // Value Functions and Rewards
    std::string vf_update_text = (update_first_vf) ? "VF1" : "VF2";
    std::string vf_non_update_text = (!update_first_vf) ? "VF1" : "VF2";
    DrawText("VALUES & REWARDS", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 5.75, GameConfigs::DEFAULT_TITLE_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    std::string previous_value_text = "PREV. STATE VALUE (" + vf_update_text + ") : % .2f";
    DrawText(TextFormat(previous_value_text.c_str(), prev_value), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 7, GameConfigs::DEFAULT_STATS_FONT_SIZE, 
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    std::string curr_max_value_text = "CURR. STATE MAX VALUE (" + vf_non_update_text + "): % .2f";
    DrawText(TextFormat(curr_max_value_text.c_str(), curr_max_value), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 8, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    std::string new_value_text = "NEW PREV. STATE VALUE (" + vf_update_text + "): % .2f";
    DrawText(TextFormat(new_value_text.c_str(), new_value), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 9, GameConfigs::DEFAULT_STATS_FONT_SIZE, 
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("REWARD: %.2f", net_reward), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 10, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));

    // Configs
    DrawText("CONFIGS", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 11.5, GameConfigs::DEFAULT_TITLE_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("EPSILON: %.2f", PolicyConfigs::EPSILON), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 12.75, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("DISC. RATE: %.2f", PolicyConfigs::DISCOUNT_RATE), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 13.75, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("LEARNING RATE: %.2f", PolicyConfigs::LEARNING_RATE), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 14.75, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("EPOCH COUNT: %i", PolicyConfigs::epochs), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 15.75, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("MAX FPS: %i", GameConfigs::MAX_FPS), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 16.75, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("MAX # STEPS: %i", PolicyConfigs::MAX_STEPS), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 17.75, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("CURRENT # STEPS: %i", PolicyConfigs::curr_steps), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 18.75, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    
    // Gas and trap count
    DrawText("INTERACTABLES", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 20.25, GameConfigs::DEFAULT_TITLE_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("ROCK COUNT: %i", rock_group.size()), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 21.5, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText(TextFormat("TRAP COUNT: %i", trap_group.size()), GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 22.5, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));

    // Keybinds
    DrawText("KEYBINDS", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 24, GameConfigs::DEFAULT_TITLE_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText("Up/Down Arrow - FPS Change", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 25.25, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText("W/S - Epsilon Change", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 26.25, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText("E/D - Learning R. Change", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 27.25, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText("C - Toggle Visited States", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 28.25, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
    DrawText("P - Toggle Visualization", GameConfigs::DEFAULT_STATS_X_POS,
        GameConfigs::DEFAULT_STATS_Y_POS + GameConfigs::DEFAULT_STATS_OFFSET * 29.25, GameConfigs::DEFAULT_STATS_FONT_SIZE,
        GetColor(GameConfigs::DEFAULT_STATS_COLOR));
}

// Custom hasher for processing position arrays
size_t GameProcesses::ArrayPositionHasher::operator()(const std::array<size_t, GameConfigs::POSITION_DIMENSIONS>& array_position) const
{
    size_t running_sum = 0;
    for (size_t i = 0; i < GameConfigs::POSITION_DIMENSIONS; ++i)
        running_sum += array_position[i] * pow(POWER_PRIME, i + 1);
    return std::hash<size_t>()(running_sum % MOD_PRIME);
}

// Generate num_positions amount of unique positions
std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, GameProcesses::ArrayPositionHasher> 
    GameProcesses::generate_unique_positions(size_t num_positions,
    size_t min_range, size_t max_range)
{
    std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, GameProcesses::ArrayPositionHasher> unique_positions;

    std::random_device rd;
    std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dis(min_range, max_range);

    for (size_t _ = 0; _ < num_positions; ++_)
    {
        std::array<size_t, GameConfigs::POSITION_DIMENSIONS> new_unique_position = 
            {static_cast<size_t>(dis(gen)), static_cast<size_t>(dis(gen))};
        while (
            // Check if state in set already
            unique_positions.find(new_unique_position) != unique_positions.end() || 
            // Spawn state check
                (new_unique_position[0] == GameConfigs::BOT_START_X_STATE && 
                    new_unique_position[1] == GameConfigs::BOT_START_Y_STATE) ||
            // Terminal state check
                (new_unique_position[0] == GameConfigs::TERMINAL_X_STATE &&
                    new_unique_position[1] == GameConfigs::TERMINAL_Y_STATE))
            new_unique_position = { static_cast<size_t>(dis(gen)), static_cast<size_t>(dis(gen)) };

        unique_positions.insert(new_unique_position);
    }

    return unique_positions;
}

// Take num_sprites from the unique_positions set, and create a vector of sprites (directly pops elements out of the unique positions set)
template<typename T>
std::vector<T> GameProcesses::generate_sprites(std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, GameProcesses::ArrayPositionHasher>& unique_positions, 
    size_t num_sprites)
{
    if (unique_positions.size() < num_sprites)
        throw std::invalid_argument("Ensure that the number of sprites requested is less than or equal to the number of positions");

    std::vector<T> group(num_sprites);
    for (size_t i = 0; i < num_sprites; ++i)
    {
        std::array<size_t, GameConfigs::POSITION_DIMENSIONS> curr_position = std::move(*unique_positions.begin());
        Vector2 unique_position = Vector2();
        unique_position.x = curr_position[0];
        unique_position.y = curr_position[1];

        group[i] = T(unique_position);

        unique_positions.erase(unique_positions.begin());
    }

    return group;
}

template <typename T>
bool GameProcesses::check_escaped_boundary(const T& sprite)
{
    return sprite.get_position().x > GameConfigs::GAME_MAX_X_STATE ||
        sprite.get_position().x < GameConfigs::GAME_MIN_X_STATE ||
        sprite.get_position().y > GameConfigs::GAME_MAX_Y_STATE ||
        sprite.get_position().y < GameConfigs::GAME_MIN_Y_STATE;
}

template <typename T>
void GameProcesses::enforce_boundary(T& sprite)
{
    Vector2 curr_position = sprite.get_position();
    if (curr_position.x > GameConfigs::GAME_MAX_X_STATE)
    {   
        curr_position.x = GameConfigs::GAME_MAX_X_STATE;
        sprite.set_position(curr_position);
    }

    if (curr_position.x < GameConfigs::GAME_MIN_X_STATE)
    {
        curr_position.x = GameConfigs::GAME_MIN_X_STATE;
        sprite.set_position(curr_position);
    }

    if (curr_position.y > GameConfigs::GAME_MAX_Y_STATE)
    {
        curr_position.y = GameConfigs::GAME_MAX_Y_STATE;
        sprite.set_position(curr_position);
    }
        
    if (curr_position.y < GameConfigs::GAME_MIN_Y_STATE)
    {
        curr_position.y = GameConfigs::GAME_MIN_Y_STATE;
        sprite.set_position(curr_position);
    }
}

template<typename T, typename S>
bool GameProcesses::check_collision_between_sprites(const T& first, const S& second)
{ return first.get_position().x == second.get_position().x && first.get_position().y == second.get_position().y; }

template<typename T, typename S>
bool GameProcesses::check_collision_between_sprite_and_group(const T& sprite, const std::vector<S>& group)
{
    for (size_t i = 0; i < group.size(); ++i)
        if (check_collision_between_sprites<T, S>(sprite, group[i]))
            return true;
    return false;
}

// Before removing the sprite at the current position, add it to a special group of removed sprites to reappend them later
template<typename T>
void GameProcesses::remove_sprite_at_state_from_group(const std::array<size_t, GameConfigs::POSITION_DIMENSIONS>& bot_state, 
    std::vector<T>& group, std::vector<T>& removed_group)
{
    if (group.empty())
        return;

    for (auto it = group.begin(); it != group.end(); ++it)
        if ((*it).get_position().x == bot_state[0] && (*it).get_position().y == bot_state[1])
        {
            removed_group.push_back(*it);
            group.erase(it);
            break;
        }
}

template<typename T>
void GameProcesses::reappend_removed_group_sprites(std::vector<T>& group, std::vector<T>& removed_group)
{
    group.insert(group.end(), removed_group.begin(), removed_group.end());
    removed_group.clear();
}

// Every instance passed into the variadic function must have a get_position function
template<typename T>
void GameProcesses::draw_sprites(const T& sprite) {
    DrawCircle(sprite.get_position().x * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_X_OFFSET,
        sprite.get_position().y * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_Y_OFFSET,
        GameConfigs::SPRITE_RADIUS, sprite.get_color());
}
template<typename T, typename... Args>
void GameProcesses::draw_sprites(const T& sprite, const Args&... rest) {
    DrawCircle(sprite.get_position().x * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_X_OFFSET,
        sprite.get_position().y * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_Y_OFFSET,
        GameConfigs::SPRITE_RADIUS, sprite.get_color());
    GameProcesses::draw_sprites(rest...);
}

template<typename T>
void GameProcesses::draw_group(const std::vector<T>& group)
{
    for (size_t i = 0; i < group.size(); ++i)
        GameProcesses::draw_sprites(group[i]);
}

void GameProcesses::draw_training_screen()
{
    DrawText("TRAINING...", GameConfigs::DEFAULT_TRAINING_SCREEN_X_POS, GameConfigs::DEFAULT_TRAINING_SCREEN_Y_POS, 
        GameConfigs::DEFAULT_TITLE_FONT_SIZE, GetColor(GameConfigs::DEFAULT_STATS_COLOR));
}