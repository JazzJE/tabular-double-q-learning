# Tabular Double Q-Learning Demonstration

A small C++17 reinforcement-learning demonstration that visualizes tabular Double Q-learning on a grid map. The agent learns to reach a terminal state while receiving rewards for collecting rocks and penalties for traps and leaving the map.

The project uses Raylib for rendering and CMake to configure the build. No external machine-learning libraries are required.

## Features

- Tabular Double Q-learning with two value functions
- Epsilon-greedy action selection
- Configurable epsilon and learning rate during runtime
- Grid-based movement with boundary enforcement
- Terminal state, collectible rocks, and traps
- Optional visualization of visited states
- Adjustable target FPS
- Basic value-function and position-generation test executable

## Requirements

- CMake 3.15 or newer
- A C++17-compatible compiler
- Git, because Raylib is downloaded through CMake `FetchContent`
- Raylib 5.5, automatically fetched during configuration

The project has been tested with the Ninja generator and MSVC on Windows.

## Build

Clone the repository:

```powershell
git clone https://github.com/JazzJE/tabular-double-q-learning.git
cd tabular-double-q-learning
```

Configure and build with Ninja:

```powershell
cmake -S . -B out/build -G Ninja
cmake --build out/build
```

The executables are generated under:

```text
out/build/bin/
```

## Run

On Windows, run the application with:

```powershell
.\out\build\bin\tabular_q_learning.exe
```

## Controls

| Key | Action |
|---|---|
| `W` / `S` | Increase or decrease epsilon |
| `E` / `D` | Increase or decrease the learning rate |
| `Up` / `Down` | Increase or decrease the target FPS |
| `C` | Toggle clearing of visited-state visualization |
| `P` | Toggle learning visualization |

Epsilon and learning rate are naturally clamped to the range `[0, 1]`.

## Configuration

Runtime and environment settings are defined in `include/policy_configs.h` and `include/game.h`, including:

- Epsilon and learning rate
- Discount rate
- Reward and penalty values
- Map dimensions
- Number of rocks and traps
- Maximum episode steps

The two value functions are stored as `std::unordered_map` instances keyed by state-action pairs.

## Project structure

```text
include/       Public headers and configuration
include/sprites/ Sprite types used by the game
src/           Application, game loop, bot, and Q-learning implementation
tests/         Basic executable tests
CMakeLists.txt Build configuration
```

## Run the tests

Build the test executable with the main target:

```powershell
cmake --build out/build --target testing
```

Run it with:

```powershell
.\out\build\bin\testing.exe
```

The current tests exercise state-action lookup/update behavior and position generation.