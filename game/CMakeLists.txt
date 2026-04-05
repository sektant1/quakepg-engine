# QuakePG Game - Executable

set(GAME_SOURCES
    src/main.cpp
    src/dungeon/dungeon_map.cpp
)

add_executable(quakepg_game ${GAME_SOURCES})

# Link against the engine - gets all public includes/libs transitively
target_link_libraries(quakepg_game PRIVATE quakepg_engine)

# Game's own headers
target_include_directories(quakepg_game
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set working directory for running from build dir
set_target_properties(quakepg_game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
