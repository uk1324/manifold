#pragma once

#include <engine/Math/Vec3.hpp>
#include <optional>

std::optional<f32> rayPlaneIntersection(Vec3 rayOrigin, Vec3 rayDirection, Vec3 pointOnPlane, Vec3 planeNormal);