#include "RayIntersection.hpp"

std::optional<f32> rayPlaneIntersection(Vec3 rayOrigin, Vec3 rayDirection, Vec3 pointOnPlane, Vec3 planeNormal) {
    // Assuming vectors are all normalized
    const auto d = dot(planeNormal, rayDirection);
    const auto epsilon = 1e-6;

    // Perpendicular
    if (abs(d) < epsilon) {
        return std::nullopt;
    }

    const auto t = dot(pointOnPlane - rayOrigin, planeNormal) / d;

    // Wrong direction
    if (t < 0.0f) {
        return std::nullopt;
    }
    return t;
}