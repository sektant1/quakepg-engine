#include <game/dungeon/dungeon_map.h>
#include <engine/core/log.h>

#include <cstring>
#include <vector>

using namespace qp;

char dungeon_map_cell(const DungeonMap& map, i32 x, i32 z) {
    if (x < 0 || x >= map.width || z < 0 || z >= map.height) return '#';
    return map.cells[z * map.width + x];
}

bool dungeon_map_is_wall(const DungeonMap& map, i32 x, i32 z) {
    return dungeon_map_cell(map, x, z) == '#';
}

Vec3 dungeon_map_cell_center(const DungeonMap& map, i32 x, i32 z) {
    float cs = map.cell_size;
    return {x * cs + cs * 0.5f, 0.0f, z * cs + cs * 0.5f};
}

// Helper: push a quad (2 triangles) into vertex/index buffers
static void push_quad(std::vector<Vertex>& verts, std::vector<u32>& idxs,
                      Vec3 p0, Vec3 p1, Vec3 p2, Vec3 p3,
                      Vec3 normal, Vec4 color,
                      float u0, float v0, float u1, float v1) {
    u32 base = (u32)verts.size();

    verts.push_back({{p0.x,p0.y,p0.z}, {u0,v0}, {normal.x,normal.y,normal.z}, {color.x,color.y,color.z,color.w}});
    verts.push_back({{p1.x,p1.y,p1.z}, {u1,v0}, {normal.x,normal.y,normal.z}, {color.x,color.y,color.z,color.w}});
    verts.push_back({{p2.x,p2.y,p2.z}, {u1,v1}, {normal.x,normal.y,normal.z}, {color.x,color.y,color.z,color.w}});
    verts.push_back({{p3.x,p3.y,p3.z}, {u0,v1}, {normal.x,normal.y,normal.z}, {color.x,color.y,color.z,color.w}});

    idxs.push_back(base + 0);
    idxs.push_back(base + 1);
    idxs.push_back(base + 2);
    idxs.push_back(base + 2);
    idxs.push_back(base + 3);
    idxs.push_back(base + 0);
}

void dungeon_map_load(DungeonMap& map, const char** rows, i32 row_count,
                      float cell_size, float wall_height) {
    map.cell_size   = cell_size;
    map.wall_height = wall_height;
    map.height      = row_count;
    map.width       = (i32)strlen(rows[0]);

    map.cells.resize(map.width * map.height);
    map.wall_colliders.clear();

    // Parse cells
    for (i32 z = 0; z < map.height; z++) {
        for (i32 x = 0; x < map.width; x++) {
            char c = rows[z][x];
            map.cells[z * map.width + x] = c;

            if (c == 'P') {
                Vec3 center = dungeon_map_cell_center(map, x, z);
                map.player_spawn = {center.x, 1.6f, center.z};
                map.cells[z * map.width + x] = '.'; // treat as open
            }
        }
    }

    LOG_INFO("Dungeon map loaded: %dx%d, spawn at (%.1f, %.1f, %.1f)",
             map.width, map.height,
             map.player_spawn.x, map.player_spawn.y, map.player_spawn.z);

    // Generate geometry
    float cs = cell_size;
    float wh = wall_height;

    std::vector<Vertex> floor_verts, wall_verts, ceil_verts;
    std::vector<u32>    floor_idxs,  wall_idxs,  ceil_idxs;

    // Color palette (stone dungeon feel)
    Vec4 floor_color   = {0.30f, 0.25f, 0.20f, 1.0f};
    Vec4 ceil_color    = {0.20f, 0.18f, 0.15f, 1.0f};
    Vec4 wall_color    = {0.40f, 0.35f, 0.28f, 1.0f};
    Vec4 wall_color_dk = {0.32f, 0.28f, 0.22f, 1.0f};

    for (i32 z = 0; z < map.height; z++) {
        for (i32 x = 0; x < map.width; x++) {
            float wx = x * cs;
            float wz = z * cs;

            if (dungeon_map_is_wall(map, x, z)) {
                // Wall collider (full cell, floor to ceiling)
                AABB col;
                col.min = {wx, 0.0f, wz};
                col.max = {wx + cs, wh, wz + cs};
                map.wall_colliders.push_back(col);

                // We don't render the top of wall blocks (hidden by ceiling)
                // Only render faces exposed to open space

                // Check each neighbor - if open, emit a wall face
                // +X face (neighbor at x+1)
                if (!dungeon_map_is_wall(map, x + 1, z)) {
                    push_quad(wall_verts, wall_idxs,
                        {wx+cs, 0,  wz},    {wx+cs, 0,  wz+cs},
                        {wx+cs, wh, wz+cs}, {wx+cs, wh, wz},
                        {1,0,0}, wall_color, 0, 0, 1, 1);
                }
                // -X face
                if (!dungeon_map_is_wall(map, x - 1, z)) {
                    push_quad(wall_verts, wall_idxs,
                        {wx, 0,  wz+cs}, {wx, 0,  wz},
                        {wx, wh, wz},    {wx, wh, wz+cs},
                        {-1,0,0}, wall_color_dk, 0, 0, 1, 1);
                }
                // +Z face
                if (!dungeon_map_is_wall(map, x, z + 1)) {
                    push_quad(wall_verts, wall_idxs,
                        {wx+cs, 0,  wz+cs}, {wx, 0,  wz+cs},
                        {wx,    wh, wz+cs}, {wx+cs, wh, wz+cs},
                        {0,0,1}, wall_color, 0, 0, 1, 1);
                }
                // -Z face
                if (!dungeon_map_is_wall(map, x, z - 1)) {
                    push_quad(wall_verts, wall_idxs,
                        {wx, 0,  wz},    {wx+cs, 0,  wz},
                        {wx+cs, wh, wz}, {wx, wh, wz},
                        {0,0,-1}, wall_color_dk, 0, 0, 1, 1);
                }

            } else {
                // Open cell: floor + ceiling

                // Floor (y=0, normal up)
                // Slight color variation based on position
                Vec4 fc = floor_color;
                if ((x + z) % 2 == 0) {
                    fc.x += 0.03f; fc.y += 0.02f; fc.z += 0.01f;
                }
                push_quad(floor_verts, floor_idxs,
                    {wx, 0, wz+cs},    {wx+cs, 0, wz+cs},
                    {wx+cs, 0, wz},    {wx, 0, wz},
                    {0,1,0}, fc, 0, 0, 1, 1);

                // Ceiling (y=wh, normal down)
                push_quad(ceil_verts, ceil_idxs,
                    {wx, wh, wz},       {wx+cs, wh, wz},
                    {wx+cs, wh, wz+cs}, {wx, wh, wz+cs},
                    {0,-1,0}, ceil_color, 0, 0, 1, 1);
            }
        }
    }

    // Build GPU meshes
    if (!floor_verts.empty())
        map.floor_mesh = mesh_create(floor_verts.data(), (u32)floor_verts.size(),
                                     floor_idxs.data(), (u32)floor_idxs.size());

    if (!wall_verts.empty())
        map.wall_mesh = mesh_create(wall_verts.data(), (u32)wall_verts.size(),
                                    wall_idxs.data(), (u32)wall_idxs.size());

    if (!ceil_verts.empty())
        map.ceiling_mesh = mesh_create(ceil_verts.data(), (u32)ceil_verts.size(),
                                       ceil_idxs.data(), (u32)ceil_idxs.size());

    LOG_INFO("Dungeon geometry: %zu floor tris, %zu wall tris, %zu ceil tris, %zu colliders",
             floor_idxs.size() / 3, wall_idxs.size() / 3,
             ceil_idxs.size() / 3, map.wall_colliders.size());
}

void dungeon_map_destroy(DungeonMap& map) {
    if (map.floor_mesh.vao)   mesh_destroy(map.floor_mesh);
    if (map.wall_mesh.vao)    mesh_destroy(map.wall_mesh);
    if (map.ceiling_mesh.vao) mesh_destroy(map.ceiling_mesh);
    map.cells.clear();
    map.wall_colliders.clear();
}
