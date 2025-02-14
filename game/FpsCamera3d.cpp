#include "FpsCamera3d.hpp"
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Window.hpp>

void FpsCamera3d::update(float dt) {
	if (Window::isCursorEnabled()) {
		lastMousePosition = std::nullopt;
	}

	Vec2 mouseOffset(0.0f);
	if (lastMousePosition.has_value()) {
		mouseOffset = Input::cursorPosWindowSpace() - *lastMousePosition;
	} else {
		mouseOffset = Vec2(0.0f);
	}

	lastMousePosition = Input::cursorPosWindowSpace();

	// x+ is right both in window space and in the used coordinate system.
	// The coordinate system is left handed so by applying the left hand rule a positive angle change turns the camera right.
	angleAroundUpAxis += mouseOffset.x * rotationSpeed * dt;
	// Down is positive in window space and a positive rotation around the x axis rotates down.
	angleAroundRightAxis += mouseOffset.y * rotationSpeed * dt;

	angleAroundUpAxis = normalizeAngleZeroToTau(angleAroundUpAxis);
	angleAroundRightAxis = std::clamp(angleAroundRightAxis, -degToRad(89.0f), degToRad(89.0f));

	Vec3 movementDirection(0.0f);

	if (Input::isKeyHeld(KeyCode::A)) movementDirection += Vec3::LEFT;
	if (Input::isKeyHeld(KeyCode::D)) movementDirection += Vec3::RIGHT;
	if (Input::isKeyHeld(KeyCode::W)) movementDirection += Vec3::FORWARD;
	if (Input::isKeyHeld(KeyCode::S)) movementDirection += Vec3::BACK;
	if (Input::isKeyHeld(KeyCode::SPACE)) movementDirection += Vec3::UP;
	if (Input::isKeyHeld(KeyCode::LEFT_SHIFT)) movementDirection += Vec3::DOWN;

	Vec3 movement(0.0f);
	movementDirection = movementDirection.normalized();

	Quat rotationAroundYAxis(angleAroundUpAxis, Vec3::UP);
	const auto dir = rotationAroundYAxis * movementDirection;
	movement += dir;


	const auto forwardMovement = (cameraForwardRotation() * Vec3::FORWARD);
	/*if (Input::isKeyHeld(KeyCode::W)) movement += forwardMovement;
	if (Input::isKeyHeld(KeyCode::S)) movement -= forwardMovement;*/

	position += movement * movementSpeed * dt;

}

Quat FpsCamera3d::cameraForwardRotation() const {
	return Quat(angleAroundUpAxis, Vec3::UP) * Quat(angleAroundRightAxis, Vec3::RIGHT);
}

Mat4 FpsCamera3d::viewMatrix() const {
	auto target = position + cameraForwardRotation() * Vec3::FORWARD;
	return Mat4::lookAt(position, target, Vec3::UP);
}
