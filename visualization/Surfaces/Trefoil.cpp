#include "Trefoil.hpp"
#include <engine/Math/Derivative.hpp>
#include "RectParametrization.hpp"

// Could scale the grid text based on the length of the curve.
Vec3 trefoilCurve(f32 t) {
	const auto n = 2;
	return Vec3(sin(t) + 2 * sin(n * t), cos(t) - 2 * cos(n * t), -sin(3 * t));
	/*return Vec3(
		(2 + cos(2.0f * t)) * cos(3.0f * t),
		(2 + cos(2.0f * t)) * sin(3.0f * t),
		sin(4.0f * t)
	);*/
}

// Theoretically it might be possible to optimize this further by calcuating sin using cos
Vec3 trefoilCurveTangent(f32 t) {
	//return derivativeMidpoint([](f32 t) { return trefoilCurve(t); }, t, step).normalized();
	return Vec3(
		sin(t) + 4.0f * cos(2.0f * t),
		(8.0f * cos(t) - 1.0f) * sin(t),
		-3.0f * cos(3.0f * t)
	).normalized();
}

Vec3 trefoilCurveNormal(f32 t) {
	// Could use the explicit formula that uses x'' instead of doing numerical differentiation of the tangent function.
	return derivativeMidpoint([](f32 t) { return trefoilCurveTangent(t); }, t, step).normalized();
}

Vec3 Trefoil::position(f32 u, f32 v) const {
	const auto r = 0.4f;
	//const auto r = 0.2f;
	const auto p = trefoilCurve(u);
	const auto tangent = trefoilCurveTangent(u);
	const auto normal = trefoilCurveNormal(u);
	const auto binormal = cross(tangent, normal);
	return p + r * (cos(v) * normal + sin(v) * binormal);
}