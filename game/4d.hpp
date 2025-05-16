#pragma once

#include <engine/Math/Vec4.hpp>

Vec4 projectVectorToSphereTangentSpace(Vec4 pointOnSphere, Vec4 anyVector);
Vec4 moveForwardOnSphere(Vec4 initialPosition, Vec4 direction);