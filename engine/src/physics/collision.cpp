#include <engine/physics/collision.h>

namespace qp {

// Move box by velocity one axis at a time, check collisions, resolve.
Vec3 aabb_slide(AABB box, Vec3 velocity, const AABB* walls, u32 wall_count) {
    Vec3 pos = {
        (box.min.x + box.max.x) * 0.5f,
        (box.min.y + box.max.y) * 0.5f,
        (box.min.z + box.max.z) * 0.5f
    };
    Vec3 half = {
        (box.max.x - box.min.x) * 0.5f,
        (box.max.y - box.min.y) * 0.5f,
        (box.max.z - box.min.z) * 0.5f
    };

    // Resolve X
    pos.x += velocity.x;
    AABB test_x = {pos - half, pos + half};
    for (u32 i = 0; i < wall_count; i++) {
        if (aabb_intersects(test_x, walls[i])) {
            if (velocity.x > 0) pos.x = walls[i].min.x - half.x;
            else                pos.x = walls[i].max.x + half.x;
            test_x = {pos - half, pos + half};
        }
    }

    // Resolve Z
    pos.z += velocity.z;
    AABB test_z = {pos - half, pos + half};
    for (u32 i = 0; i < wall_count; i++) {
        if (aabb_intersects(test_z, walls[i])) {
            if (velocity.z > 0) pos.z = walls[i].min.z - half.z;
            else                pos.z = walls[i].max.z + half.z;
            test_z = {pos - half, pos + half};
        }
    }

    // Resolve Y
    pos.y += velocity.y;
    AABB test_y = {pos - half, pos + half};
    for (u32 i = 0; i < wall_count; i++) {
        if (aabb_intersects(test_y, walls[i])) {
            if (velocity.y > 0) pos.y = walls[i].min.y - half.y;
            else                pos.y = walls[i].max.y + half.y;
            test_y = {pos - half, pos + half};
        }
    }

    return pos;
}

} // namespace qp
