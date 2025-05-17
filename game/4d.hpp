#pragma once

#include <engine/Math/Vec4.hpp>
#include <engine/Math/Mat4.hpp>

Vec4 quatMul(Vec4 a, Vec4 b);

Vec4 projectVectorToSphereTangentSpace(Vec4 pointOnSphere, Vec4 anyVector);
Vec4 moveForwardOnSphere(Vec4 initialPosition, Vec4 direction);
// This doesn't correctly transform vectors that aren't tangent to the geodesic. It rotates them. That is it doesn't do parallel transport. Use the matrix version for parallel transport.
Vec4 movementForwardOnSphereQuick(Vec4 initialPosition, Vec4 direction);
Mat4 movementForwardOnSphere(Vec4 initialPosition, Vec4 direction);
Vec4 normalizedDirectionFromAToB(Vec4 a, Vec4 b);
f32 parallelogramArea(Vec4 v0, Vec4 v1);
f32 parallelepipedArea(Vec4 v0, Vec4 v1, Vec4 v2);
f32 distanceFromPlaneToPoint(Vec4 planePoint, Vec4 planeSpanning0, Vec4 planeSpanning1, Vec4 point);