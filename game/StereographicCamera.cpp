#include "StereographicCamera.hpp"
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Window.hpp>

static Quat exp(Vec3 vectorPart) {
	const auto distance = vectorPart.length();
	const auto v = distance == 0.0f ? Vec3(0.0f) : vectorPart / distance;
	return Quat(v.x * sin(distance), v.y * sin(distance), v.z * sin(distance), cos(distance));
}
#include <imgui/imgui.h>
Quat StereographicCamera::position() const {
	return -p.inverseIfNormalized();
}
void StereographicCamera::update(float dt) {
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


	//// x+ is right both in window space and in the used coordinate system.
	//// The coordinate system is left handed so by applying the left hand rule a positive angle change turns the camera right.
	//angleAroundUpAxis += mouseOffset.x * rotationSpeed * dt;
	//// Down is positive in window space and a positive rotation around the x axis rotates down.
	//angleAroundRightAxis += mouseOffset.y * rotationSpeed * dt;
	Quat rotation = Quat::identity;
	rotation *= Quat(mouseOffset.x * rotationSpeed * dt, up);
	rotation *= Quat(mouseOffset.y * rotationSpeed * dt, right);

	right *= rotation;
	up *= rotation;

	/*angleAroundUpAxis = normalizeAngleZeroToTau(angleAroundUpAxis);
	angleAroundRightAxis = std::clamp(angleAroundRightAxis, -degToRad(89.0f), degToRad(89.0f));*/

	Vec3 movementDirection(0.0f);

	if (Input::isKeyHeld(KeyCode::A)) movementDirection += Vec3::LEFT;
	if (Input::isKeyHeld(KeyCode::D)) movementDirection += Vec3::RIGHT;
	if (Input::isKeyHeld(KeyCode::W)) movementDirection += Vec3::FORWARD;
	if (Input::isKeyHeld(KeyCode::S)) movementDirection += Vec3::BACK;
	if (Input::isKeyHeld(KeyCode::SPACE)) movementDirection += Vec3::UP;
	if (Input::isKeyHeld(KeyCode::LEFT_SHIFT)) movementDirection += Vec3::DOWN;

	Vec3 movement(0.0f);
	movementDirection = movementDirection.normalized();

	/*Quat rotationAroundYAxis(angleAroundUpAxis, Vec3::UP);
	const auto dir = rotationAroundYAxis * movementDirection;*/
	
	const auto f = forward();

	movement += right * movementDirection.x + up * movementDirection.y + f * movementDirection.z;
	ImGui::Text("stereographic movement basis det: %g", dot(cross(up, right), f));
	const auto nR = cross(up, f).normalized();
	const auto nU = cross(f, nR).normalized();
	right = nR;
	up = nU;
	
	//const auto forwardMovement = forward();
	/*if (Input::isKeyHeld(KeyCode::W)) movement += forwardMovement;
	if (Input::isKeyHeld(KeyCode::S)) movement -= forwardMovement;*/
	p = exp(movement * movementSpeed * dt) * p;
	p = p.normalized();

	testP = exp(movement * (-testP * movementSpeed) * dt) * testP;
	testP = testP.normalized();
}

//Quat StereographicCamera::cameraForwardRotation() const {
//	return Quat(angleAroundUpAxis, Vec3::UP) * Quat(angleAroundRightAxis, Vec3::RIGHT);
//}

Vec3 StereographicCamera::forward() const {
	return cross(right, up);
	//return cameraForwardRotation() * Vec3::FORWARD;
}

Mat4 StereographicCamera::viewMatrix() const {
	auto target = pos3d() + forward();
	/*return Mat4::lookAt(pos3d(), target, Vec3::UP);*/
	return Mat4::lookAt(pos3d(), target, up);
}

Vec3 StereographicCamera::pos3d() const {
	return Vec3(0.0f);
}
