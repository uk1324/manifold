#pragma once

#include <engine/Math/Vec3.hpp>
#include <engine/Math/Vec2.hpp>
#include <optional>

Vec3 triCenter(Vec3 v0, Vec3 v1, Vec3 v2);
Vec3 triCenter(const Vec3* v);

struct RayTriIntersection {
    Vec3 position;
    Vec3 barycentricCoordinates;
    f32 t;
};

std::optional<RayTriIntersection> rayTriIntersection(
    Vec3 rayOrigin,
    Vec3 rayDirection,
    Vec3 v0,
    Vec3 v1,
    Vec3 v2);

std::optional<RayTriIntersection> rayTriIntersection(
    Vec3 rayOrigin,
    Vec3 rayDirection,
    const Vec3* v);

template<typename T>
T barycentricInterpolate(Vec3 barycentricCoordinates, const T& v0, const T& v1, const T& v2) {
    return 
        barycentricCoordinates.x * v0 + 
        barycentricCoordinates.y * v1 + 
        barycentricCoordinates.z * v2;
}

template<typename T>
T barycentricInterpolate(Vec3 barycentricCoordinates, const T* values) {
    return barycentricInterpolate(barycentricCoordinates, values[0], values[1], values[2]);
}

f32 triArea(Vec3 v0, Vec3 v1, Vec3 v2);
f32 triArea(const Vec3* v);

// r0 and r1 are uniformly distribiuted on [0, 1]
Vec3 uniformRandomPointOnTri(Vec3 v0, Vec3 v1, Vec3 v2, f32 r0, f32 r1);
Vec3 uniformRandomPointOnTri(const Vec3* v, f32 r0, f32 r1);

Vec2 uniformRandomPointOnTri(Vec2 v0, Vec2 v1, Vec2 v2, f32 r0, f32 r1);
Vec2 uniformRandomPointOnTri(const Vec2* v, f32 r0, f32 r1);