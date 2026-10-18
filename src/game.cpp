#include <unordered_set>
#include <set>
#include <array>
#include <random>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "bot_sprite.h"
#include "gas_sprite.h"
#include "trap_sprite.h"
#include "game.h"
#include "raylib.h"

void GameProcesses::run()
{
    // Create a 2D array of all possible positions for the map
    InitWindow(GameConfigs::SCREEN_WIDTH, GameConfigs::SCREEN_HEIGHT, "Double Q-Learning Demonstration");
    SetTargetFPS(60);

    BotSprite bot;
    
    std::unordered_set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>, GameProcesses::ArrayPositionHasher> 
        unique_positions = GameProcesses::generate_unique_positions(GameConfigs::NUM_GAS + GameConfigs::NUM_SPIKES);

    // Create a vector of all the gas sprites
    std::vector<GasSprite> gas_group = generate_sprites<GasSprite>(unique_positions, GameConfigs::NUM_GAS);
    // Create a vector of all the trap sprites
    std::vector<TrapSprite> trap_group = generate_sprites<TrapSprite>(unique_positions, GameConfigs::NUM_SPIKES);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (IsKeyDown(KEY_UP))
        {
            bot.take_action(PolicyConfigs::UP);
        }
        if (IsKeyDown(KEY_DOWN))
        {
            bot.take_action(PolicyConfigs::DOWN);
        }
        if (IsKeyDown(KEY_LEFT))
        {
            bot.take_action(PolicyConfigs::LEFT);
        }
        if (IsKeyDown(KEY_RIGHT))
        {
            bot.take_action(PolicyConfigs::RIGHT);
        }

        // Drawing sprites to screen
        GameProcesses::draw_visited_states(bot);
        GameProcesses::draw_group(gas_group);
        GameProcesses::draw_group(trap_group);
        GameProcesses::draw_sprites(bot);
        GameProcesses::draw_map();

        std::cout << std::endl << "X POS : " << bot.get_position().x << " Y POS : " << bot.get_position().y;
        std::cout << std::endl << GameProcesses::check_collision_between_sprite_and_group(bot, gas_group);
        std::cout << std::endl << GameProcesses::check_collision_between_sprite_and_group(bot, trap_group);

        EndDrawing();
    }

    CloseWindow();
    return;
}

void GameProcesses::draw_map()
{
    // Draw vertical lines
    for (size_t s = 0; s * GameConfigs::TILE_SIZE <= GameConfigs::SCREEN_WIDTH - GameConfigs::GUI_WIDTH; ++s)
        DrawLine(s * GameConfigs::TILE_SIZE, 0, s * GameConfigs::TILE_SIZE, GameConfigs::SCREEN_HEIGHT, 
            GetColor(GameConfigs::TILE_BORDER_COLOR));

    // Draw horizontal lines
    for (size_t s = 0; s * GameConfigs::TILE_SIZE <= GameConfigs::SCREEN_HEIGHT; ++s)
        DrawLine(0, s * GameConfigs::TILE_SIZE, GameConfigs::SCREEN_WIDTH - GameConfigs::GUI_WIDTH, s * GameConfigs::TILE_SIZE,
            GetColor(GameConfigs::TILE_BORDER_COLOR));
}

// Fill each state visited with a circle
void GameProcesses::draw_visited_states(const Sprite& bot, bool reset_states)
{
    static std::set<std::array<size_t, GameConfigs::POSITION_DIMENSIONS>> visited_states;

    std::array<size_t, GameConfigs::POSITION_DIMENSIONS> curr_position = { bot.get_position().x, bot.get_position().y };
    visited_states.insert(curr_position);

    for (auto it = visited_states.begin(); it != visited_states.end(); ++it)
        DrawCircle((*it)[0] * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_X_OFFSET,
            (*it)[1] * GameConfigs::TILE_SIZE + GameConfigs::TILE_SIZE / 2.0 + GameConfigs::SPRITE_Y_OFFSET,
            GameConfigs::SPRITE_RADIUS, GetColor(GameConfigs::DEFAULT_VISITED_STATE_COLOR));

    if (reset_states)
        visited_states.clear();
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
    std::uniform_real_distribution<double> dis(0, GameConfigs::GAME_MAX_Y_STATE);

    for (size_t _ = 0; _ < num_positions; ++_)
    {
        std::array<size_t, GameConfigs::POSITION_DIMENSIONS> new_unique_position = 
            {static_cast<size_t>(dis(gen)), static_cast<size_t>(dis(gen))};
        while (unique_positions.find(new_unique_position) != unique_positions.end() || 
                (new_unique_position[0] == GameConfigs::BOT_START_X_STATE && new_unique_position[1] == GameConfigs::BOT_START_Y_STATE))
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