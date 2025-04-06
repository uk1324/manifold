#pragma once

#include <engine/Math/Vec3.hpp>

Vec3 derivBezier(const Vec3 P[4], f32 t);
Vec3 evalBezierCurve(const Vec3 P[4], f32 t);