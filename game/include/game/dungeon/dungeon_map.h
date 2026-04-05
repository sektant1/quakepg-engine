#pragma once

#include <engine/core/types.h>
#include <engine/core/math.h>
#include <engine/renderer/mesh.h>
#include <engine/physics/collision.h>

#include <vector>

// Dungeon map built from ASCII art.
//   # = wall
//   . = open floor
//   P = player spawn (also open floor)
//
// Each cell is CELL_SIZE x CELL_SIZE world units.
// Walls are WALL_HEIGHT tall.
// World origin (0,0,0) is at the top-left corner of the map at floor level.

struct DungeonMap {
    // Config
    float cell_size   = 3.0f;
    float wall_height = 4.0f;

    // Parsed data
    qp::i32 width  = 0;
    qp::i32 height = 0;
    std::vector<char> cells;   // row-major: cells[z * width + x]

    // Player spawn in world coords
    qp::Vec3 player_spawn = {0, 0, 0};

    // Generated geometry
    qp::Mesh floor_mesh = {};
    qp::Mesh wall_mesh  = {};
    qp::Mesh ceiling_mesh = {};

    // Collision
    std::vector<qp::AABB> wall_colliders;
};

// Load from array of C-strings (one per row). Rows must all be the same length.
void dungeon_map_load(DungeonMap& map, const char** rows, qp::i32 row_count,
                      float cell_size, float wall_height);

// Free GPU meshes
void dungeon_map_destroy(DungeonMap& map);

// Query cell type
char dungeon_map_cell(const DungeonMap& map, qp::i32 x, qp::i32 z);
bool dungeon_map_is_wall(const DungeonMap& map, qp::i32 x, qp::i32 z);

// World position of a cell center (floor level y=0)
qp::Vec3 dungeon_map_cell_center(const DungeonMap& map, qp::i32 x, qp::i32 z);
