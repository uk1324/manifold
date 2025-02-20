#include "RayIntersection.hpp"

Vec3 rayAt(f32 t, Vec3 rayOrigin, Vec3 rayDirection) {
    return rayOrigin + t * rayDirection;
}

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

std::optional<f32> rayPlaneIntersection(const Ray& ray, Vec3 pointOnPlane, Vec3 planeNormal) {
    return rayPlaneIntersection(ray.origin, ray.direction, pointOnPlane, planeNormal);
}

std::optional<f32> raySphereIntersection(Vec3 rayOrigin, Vec3 rayDirection, Vec3 sphereCenter, f32 sphereRadius) {
    const auto oc = rayOrigin - sphereCenter;
    const auto a = dot(rayDirection, rayDirection);
    const auto half_b = dot(oc, rayDirection);
    const auto c = dot(oc, oc) - sphereRadius * sphereRadius;
    const auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0.0) {
        return std::nullopt;
    }

    const auto sqrt_discriminant = sqrt(discriminant);

    const auto root = (-half_b - sqrt_discriminant) / a;

    if (root < 0.0f) {
        return std::nullopt;
    }
    return root;
}

std::optional<f32> raySphereIntersection(const Ray& ray, Vec3 sphereCenter, f32 sphereRadius) {
    return raySphereIntersection(ray.origin, ray.direction, sphereCenter, sphereRadius);
}

Ray::Ray(Vec3 origin, Vec3 direction)
    : origin(origin)
    , direction(direction) {}

Vec3 Ray::at(f32 t) const {
    return rayAt(t, origin, direction);
}
