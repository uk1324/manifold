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
//Quat StereographicCamera::position() const {
//	return -p.inverseIfNormalized();
//}
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
	Vec3 movementDirection(0.0f);

	if (Input::isKeyHeld(KeyCode::A)) movementDirection += Vec3::LEFT;
	if (Input::isKeyHeld(KeyCode::D)) movementDirection += Vec3::RIGHT;
	if (Input::isKeyHeld(KeyCode::W)) movementDirection += Vec3::FORWARD;
	if (Input::isKeyHeld(KeyCode::S)) movementDirection += Vec3::BACK;
	if (Input::isKeyHeld(KeyCode::SPACE)) movementDirection += Vec3::UP;
	if (Input::isKeyHeld(KeyCode::LEFT_SHIFT)) movementDirection += Vec3::DOWN;

	movementDirection = movementDirection.normalized();

	// Only the case of being at the origin (identity of quaterions = (0, 0, 0, 1)) is considered. The other cases are done by applying these transformation with a change of basis.

	// In analogy with lower dimensions movement on a sphere it should transform the geodesics circle to to itself and it would also make sense to leave everything else invariant.
	// If we just consider the 2d subspace then we can derive a formula for the rotation using 2 reflections. Thinking about a 2d subace we need 2 lines that are at an angle half of the angle between the initial point and the target point. The line at the start has the normal movementDirection and the line between those 2 points has coordiantes (p0 - t)  (draw a picture). Then these lines extend to hyperplanes. The reflection is going to only change points on the geodesics circle plane and leave everyhing else fixed.
	// The formula for this rotation is derived here.
	// "Gaming in Elliptic Geometry" https://cg.iit.bme.hu/~szirmay/EllipticGames2.pdf
	// This is probably what the parallel transport is on the 3 sphere. 
	// Another way to find the transformation would be to define what properies it should have and then find what values it should have on at least 3 vectors. The 4th one has to be orthogonal to the other 3. The map should map the identity to the target point. It should also map the velocity of the curve r(t) = exp(t dir) at zero to the velocity at target. It should probably also fix a point, because we could consider a 3d subspace and it would intuitively make sense that on that 3 sphere the axis is fixed. If one subspace is fixed then another has to be as well. So the transformation should have properies:
	/*
	1:
	map (0, 0, 0, 1) to q
	2:
	r'(t) = dir exp(t dir)
	Map dir to dir * exp(t dir)
	3: Fix the 2d subspace perpendicular to these 2.

	I think the 2 reflection transformation satisfies these properties, because 
	it satisfies 1, it satisfies 2 because if we consider then 2d subspace the initial velocity vector gets mapped to the vector after movement.
	it does fix a 2d subspace perpendicular to the geodesic plane, because 2 reflection fix the subspace that is the intersection of the hyperlanes, which is a 2d subspace.
	*/
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

	// Rotations should fix (0, 0, 0, 1) so the last column is identity. Apart from that they should just rotate the 3d tangent space.
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

	//const auto t0 = rotate * rotate.transpose();
	//const auto t1 = move * move.transpose();
	//const auto t2 = transformation * transformation.transpose();
	//if (abs(t2[1][0]) > 0.02f) {
	//	int x = 5;
	//}
	/*
	If we have for example m r where m is a reflectio and r is a reflection
	It can be interpreted as rotate then mirror in the global frame or as mirror and then rotate in the mirrored frame.
	
	That is we have 
	(m r m^-1) m = m r
	(m r m^-1) means rotate in the mirrored frame and m means mirror. So we first mirror and then rotate in the mirrored frame.

	So in this case we have transformation and then in the basis of the transformed position we move and roate.
	*/
	transformation = transformation * move;
	transformation = transformation * rotate;
	gramSchmidtOrthonormalize(view(transformation.basis));
}

// The view matrix part is done in 4D already so this just returns the identity and 0 position.
Mat4 StereographicCamera::viewMatrix() const {
	return Mat4::identity;
}


Vec3 StereographicCamera::pos3d() const {
	return Vec3(0.0f);
}

Vec3 StereographicCamera::forward3d() const {
	return Vec3(0.0f, 0.0f, 1.0f);
}

Mat4 StereographicCamera::view4() const {
	return transformation.inversed();
}

Mat4 StereographicCamera::view4Inversed() const {
	return transformation;
}

Vec4 StereographicCamera::pos4() const {
	// This is just extracting the 4th column of the matrix.
	return transformation * Vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
