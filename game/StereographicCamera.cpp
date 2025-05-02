#include "StereographicCamera.hpp"
#include <engine/Input/Input.hpp>
#include <engine/Math/Angles.hpp>
#include <engine/Window.hpp>

static Quat exp(Vec3 vectorPart) {
	const auto distance = vectorPart.length();
	const auto v = distance == 0.0f ? Vec3(0.0f) : vectorPart / distance;
	return Quat(v.x * sin(distance), v.y * sin(distance), v.z * sin(distance), cos(distance));
}

static Vec3 log(Quat unitQuat) {
	return Vec3(unitQuat.x, unitQuat.y, unitQuat.z).normalized() * acos(unitQuat.w);
	/*const auto distance = vectorPart.length();
	const auto v = distance == 0.0f ? Vec3(0.0f) : vectorPart / distance;
	return Quat(v.x * sin(distance), v.y * sin(distance), v.z * sin(distance), cos(distance));*/
}
#include <engine/Math/GramSchmidt.hpp>
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

	//testP = exp(movement * (testP * movementSpeed) * dt) * testP;
	//testP = testP.normalized();
	// using the preforimg operation is basis interpretation of left multiplication
	//const auto q = 
	const auto dir = movementDirection;
	const auto target = exp(dir * movementSpeed * dt);
	const auto& q = target;
	const auto s = 1.0f / (1.0f + q.w);
	const auto m00 = 1.0f - (q.x * q.x) * s;
	const auto m11 = 1.0f - (q.y * q.y) * s;
	const auto m22 = 1.0f - (q.z * q.z) * s;
	const auto m01 = -q.y * q.x * s;
	const auto& m10 = m01;
	const auto m20 = -q.z * q.x * s;
	const auto& m02 = m20;
	const auto m12 = -q.z * q.y * s;
	const auto& m21 = m12;

	Mat4 move(
		Vec4(m00, m10, m20, q.x),
		Vec4(m01, m11, m21, q.y),
		Vec4(m02, m12, m22, q.z),
		Vec4(-q.x, q.y, q.z, q.w)
	);

	Quat rot = Quat::identity;
	rot *= Quat(mouseOffset.x * rotationSpeed * dt, Vec3(0.0f, 1.0f, 0.0f));
	rot *= Quat(mouseOffset.y * rotationSpeed * dt, Vec3(1.0f, 0.0f, 0.0f));
	const auto rM = rot.toMatrix();

	Mat4 rotate(
		Vec4(rM(0, 0), rM(1, 0), rM(2, 0), 0.0f),
		Vec4(rM(0, 1), rM(1, 1), rM(2, 1), 0.0f),
		Vec4(rM(0, 2), rM(1, 2), rM(2, 2), 0.0f),
		Vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	const auto t0 = rotate * rotate.transpose();
	const auto t1 = move * move.transpose();
	const auto t2 = transformation * transformation.transpose();
	if (abs(t2[1][0]) > 0.02f) {
		int x = 5;
	}
	/*
	If we have for example m r where m is a reflectio and r is a reflection
	It can be interpreted as rotate then mirror in the global frame or as mirror and then rotate in the mirrored frame.
	
	That is we have 
	(m r m^-1) m = m r
	(m r m^-1) means rotate in the mirrored frame and m means mirror. So we first mirror and then rotate in the mirrored frame.

	*/
	/*transformation = move * transformation;
	transformation = rotate * transformation;*/
	transformation = transformation * move;
	transformation = transformation * rotate;
	gramSchmidtOrthonormalize(view(transformation.basis));
}

//Quat StereographicCamera::cameraForwardRotation() const {
//	return Quat(angleAroundUpAxis, Vec3::UP) * Quat(angleAroundRightAxis, Vec3::RIGHT);
//}

Vec3 StereographicCamera::forward() const {
	return cross(right, up);
	//return cameraForwardRotation() * Vec3::FORWARD;
}

Mat4 StereographicCamera::viewMatrix() const {
	auto target = pos3d() + Vec3(0.0f, 0.0f, 1.0f);
	/*return Mat4::lookAt(pos3d(), target, Vec3::UP);*/
	return Mat4::lookAt(pos3d(), target, Vec3(0.0f, 1.0f, 0.0f));

	//auto target = pos3d() + forward();
	///*return Mat4::lookAt(pos3d(), target, Vec3::UP);*/
	//return Mat4::lookAt(pos3d(), target, up);
}

Vec3 StereographicCamera::pos3d() const {
	return Vec3(0.0f);
}
