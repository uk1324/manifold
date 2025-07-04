#pragma once
#include <engine/Math/Quat.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Mat4.hpp>
#include <optional> 
/*
If I am at identity and some forward vector in stereographic space then to move in that direction i have to evaluate the jacobian of the inverse stereographic projection at zero on the forward vector. This jacobian is equal to 2 * I and an empty row at the bottom so in the tangent space of the 3 sphere at identity the velocity is the same.


If we have a curve starting from p in direction v then it has the form r(t) = p exp(t v)

The issue with the method I don't know why it works is that it rotates (rolls around the forward axis) the camera when you move forward. 
*/
struct StereographicCamera {
	std::optional<Vec2> lastMousePosition;

	Mat4 transformation = Mat4::identity;

	// Angle change per pixel.
	float rotationSpeed = 0.1f;
	float movementSpeed = 1.0f;

	void update(float dt);
	Mat4 viewMatrix() const;
	Vec3 pos3d() const;
	Vec3 forward3d() const;

	// Moves the cameraPosition to (0, 0, 0, 1), forward to (0, 0, 1, 0), up to (0, 1, 0, 0) and right to (1, 0, 0, 0). It serves the same purpose as the normal view matrix. That is it moves everyhing to the origin before rendering.
	// This changes from frame to frame even if there is no movement, because it tries to renomralize it each frame.
	Mat4 view4() const;

	Mat4 view4Inversed() const;
	Vec4 pos4() const;
};