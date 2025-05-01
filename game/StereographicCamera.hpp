#pragma once
#include <engine/Math/Quat.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Mat4.hpp>
#include <optional> 

struct StereographicCamera {
	std::optional<Vec2> lastMousePosition;
	Quat position = Quat::identity;
	Vec3 right = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
	/*float angleAroundUpAxis = 0.0f;
	float angleAroundRightAxis = 0.0f;*/

	// Angle change per pixel.
	float rotationSpeed = 0.1f;
	float movementSpeed = 1.0f;

	void update(float dt);
	//Quat cameraForwardRotation() const;
	Vec3 forward() const;
	Mat4 viewMatrix() const;
	Vec3 pos3d() const;
};