#pragma once

#include <engine/Math/Vec3.hpp>
#include <optional>

struct Ray {
	Ray(Vec3 origin, Vec3 direction);
	Vec3 origin;
	Vec3 direction;

	Vec3 at(f32 t) const;
};

Vec3 rayAt(f32 t, Vec3 rayOrigin, Vec3 rayDirection);

std::optional<f32> rayPlaneIntersection(Vec3 rayOrigin, Vec3 rayDirection, Vec3 pointOnPlane, Vec3 planeNormal);
std::optional<f32> rayPlaneIntersection(const Ray& ray, Vec3 pointOnPlane, Vec3 planeNormal);
std::optional<f32> raySphereIntersection(Vec3 rayOrigin, Vec3 rayDirection, Vec3 sphereCenter, f32 sphereRadius);
std::optional<f32> raySphereIntersection(const Ray& ray, Vec3 sphereCenter, f32 sphereRadius);