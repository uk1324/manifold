#pragma once

#include <engine/Math/Vec3.hpp>
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
