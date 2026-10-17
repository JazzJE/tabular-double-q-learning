cmake_minimum_required(VERSION 3.15)

project(tabular_q_learning)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# External libraries

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

# Main application assembling

set(Q_LEARNING_SOURCES
    src/main.cpp
    src/game.cpp
    src/tabular_q.cpp
    src/bot_sprite.cpp
)

add_executable(tabular_q_learning
    ${Q_LEARNING_SOURCES}
)

# Testing assembling

set(TEST_SOURCES
    src/tabular_q.cpp
    src/game.cpp
    src/bot_sprite.cpp
)

add_executable(testing
    tests/main.cpp
    ${TEST_SOURCES}
)

target_include_directories(tabular_q_learning PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sprites
)

target_link_libraries(tabular_q_learning PRIVATE
    raylib
)

target_include_directories(testing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/sprites
)

target_link_libraries(testing PRIVATE
    raylib
)
