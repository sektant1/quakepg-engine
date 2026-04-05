#pragma once

#include <engine/core/types.h>
#include <cmath>

namespace qp {

// ---- Vec2 ----
struct Vec2 {
    f32 x, y;
};

inline Vec2 vec2(f32 x, f32 y) { return {x, y}; }

inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x+b.x, a.y+b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x-b.x, a.y-b.y}; }
inline Vec2 operator*(Vec2 a, f32 s)  { return {a.x*s, a.y*s}; }
inline Vec2 operator*(f32 s, Vec2 a)  { return {a.x*s, a.y*s}; }

// ---- Vec3 ----
struct Vec3 {
    f32 x, y, z;
};

inline Vec3 vec3(f32 x, f32 y, f32 z) { return {x, y, z}; }
inline Vec3 vec3(f32 s)               { return {s, s, s}; }

inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
inline Vec3 operator*(Vec3 a, f32 s)  { return {a.x*s, a.y*s, a.z*s}; }
inline Vec3 operator*(f32 s, Vec3 a)  { return {a.x*s, a.y*s, a.z*s}; }
inline Vec3 operator-(Vec3 a)         { return {-a.x, -a.y, -a.z}; }

inline Vec3& operator+=(Vec3& a, Vec3 b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; return a; }
inline Vec3& operator-=(Vec3& a, Vec3 b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; return a; }
inline Vec3& operator*=(Vec3& a, f32 s)  { a.x*=s; a.y*=s; a.z*=s; return a; }

inline f32  vec3_dot(Vec3 a, Vec3 b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline f32  vec3_length_sq(Vec3 a)     { return vec3_dot(a, a); }
inline f32  vec3_length(Vec3 a)        { return sqrtf(vec3_length_sq(a)); }

inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

inline Vec3 vec3_normalize(Vec3 a) {
    f32 len = vec3_length(a);
    if (len < 1e-8f) return {0, 0, 0};
    f32 inv = 1.0f / len;
    return {a.x*inv, a.y*inv, a.z*inv};
}

inline Vec3 vec3_lerp(Vec3 a, Vec3 b, f32 t) {
    return a + (b - a) * t;
}

// ---- Vec4 ----
struct Vec4 {
    f32 x, y, z, w;
};

inline Vec4 vec4(f32 x, f32 y, f32 z, f32 w) { return {x, y, z, w}; }
inline Vec4 vec4(Vec3 v, f32 w)               { return {v.x, v.y, v.z, w}; }

// ---- Mat4 (column-major, OpenGL convention) ----
// m[col][row] stored as flat array: data[col*4 + row]
struct Mat4 {
    f32 data[16];

    f32& at(i32 col, i32 row)       { return data[col * 4 + row]; }
    f32  at(i32 col, i32 row) const { return data[col * 4 + row]; }
};

inline Mat4 mat4_identity() {
    Mat4 m = {};
    m.data[0]  = 1.0f;
    m.data[5]  = 1.0f;
    m.data[10] = 1.0f;
    m.data[15] = 1.0f;
    return m;
}

inline Mat4 mat4_mul(const Mat4& a, const Mat4& b) {
    Mat4 r = {};
    for (i32 c = 0; c < 4; c++) {
        for (i32 row = 0; row < 4; row++) {
            f32 sum = 0.0f;
            for (i32 k = 0; k < 4; k++) {
                sum += a.data[k * 4 + row] * b.data[c * 4 + k];
            }
            r.data[c * 4 + row] = sum;
        }
    }
    return r;
}

inline Mat4 operator*(const Mat4& a, const Mat4& b) { return mat4_mul(a, b); }

inline Mat4 mat4_translate(Vec3 t) {
    Mat4 m = mat4_identity();
    m.data[12] = t.x;
    m.data[13] = t.y;
    m.data[14] = t.z;
    return m;
}

inline Mat4 mat4_scale(Vec3 s) {
    Mat4 m = {};
    m.data[0]  = s.x;
    m.data[5]  = s.y;
    m.data[10] = s.z;
    m.data[15] = 1.0f;
    return m;
}

inline Mat4 mat4_rotate_x(f32 radians) {
    f32 c = cosf(radians), s = sinf(radians);
    Mat4 m = mat4_identity();
    m.data[5]  =  c; m.data[9]  = -s;
    m.data[6]  =  s; m.data[10] =  c;
    return m;
}

inline Mat4 mat4_rotate_y(f32 radians) {
    f32 c = cosf(radians), s = sinf(radians);
    Mat4 m = mat4_identity();
    m.data[0]  =  c; m.data[8]  =  s;
    m.data[2]  = -s; m.data[10] =  c;
    return m;
}

inline Mat4 mat4_rotate_z(f32 radians) {
    f32 c = cosf(radians), s = sinf(radians);
    Mat4 m = mat4_identity();
    m.data[0] =  c; m.data[4] = -s;
    m.data[1] =  s; m.data[5] =  c;
    return m;
}

inline Mat4 mat4_perspective(f32 fov_radians, f32 aspect, f32 near, f32 far) {
    f32 tan_half = tanf(fov_radians * 0.5f);
    Mat4 m = {};
    m.data[0]  = 1.0f / (aspect * tan_half);
    m.data[5]  = 1.0f / tan_half;
    m.data[10] = -(far + near) / (far - near);
    m.data[11] = -1.0f;
    m.data[14] = -(2.0f * far * near) / (far - near);
    return m;
}

inline Mat4 mat4_lookat(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3_normalize(center - eye);
    Vec3 r = vec3_normalize(vec3_cross(f, up));
    Vec3 u = vec3_cross(r, f);

    Mat4 m = mat4_identity();
    m.data[0]  =  r.x; m.data[4]  =  r.y; m.data[8]  =  r.z;
    m.data[1]  =  u.x; m.data[5]  =  u.y; m.data[9]  =  u.z;
    m.data[2]  = -f.x; m.data[6]  = -f.y; m.data[10] = -f.z;
    m.data[12] = -vec3_dot(r, eye);
    m.data[13] = -vec3_dot(u, eye);
    m.data[14] =  vec3_dot(f, eye);
    return m;
}

// ---- Utility ----
constexpr f32 QP_PI = 3.14159265358979323846f;

inline f32 to_radians(f32 degrees) { return degrees * (QP_PI / 180.0f); }
inline f32 to_degrees(f32 radians) { return radians * (180.0f / QP_PI); }

inline f32 clampf(f32 v, f32 lo, f32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

} // namespace qp
