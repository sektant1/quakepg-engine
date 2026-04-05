#pragma once

#include <engine/core/types.h>
#include <engine/core/math.h>

namespace qp {

struct AABB {
    Vec3 min;
    Vec3 max;
};

inline AABB aabb_from_center_size(Vec3 center, Vec3 size) {
    Vec3 half = size * 0.5f;
    return {center - half, center + half};
}

inline bool aabb_intersects(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

// Slide collision: try to move 'box' by 'velocity', resolving against walls.
// Returns the resolved position.
Vec3 aabb_slide(AABB box, Vec3 velocity, const AABB* walls, u32 wall_count);

} // namespace qp
