#include "Trefoil.hpp"
#include <engine/Math/Derivative.hpp>
#include "RectParametrization.hpp"

const auto step = 0.05f;

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


Vec3 Trefoil::tangentU(f32 u, f32 v) const {
	return derivativeMidpoint([&v, this](f32 u) { return position(u, v); }, u, step);
}

Vec3 Trefoil::tangentV(f32 u, f32 v) const {
	return derivativeMidpoint([&u, this](f32 v) { return position(u, v); }, v, step);
}

Vec3 Trefoil::xUu(f32 u, f32 v) const {
	return secondDerivativeMidpoint([this, &v](f32 u) { return position(u, v); }, u, step);
}

Vec3 Trefoil::xVv(f32 u, f32 v) const {
	return secondDerivativeMidpoint([this, &u](f32 v) { return position(u, v); }, v, step);
}

Vec3 Trefoil::xUv(f32 u, f32 v) const {
	return mixedDerivativeMidpoint([this](f32 u, f32 v) { return position(u, v); }, u, v, step, step);
}

Mat2 Trefoil::firstFundamentalForm(f32 u, f32 v) const {
	return ::firstFundamentalForm(tangentU(u, v), tangentV(u, v));
}

Mat2 Trefoil::secondFundamentalForm(f32 u, f32 v) const {
	return ::secondFundamentalForm(xUu(u, v), xUv(u, v), xVv(u, v), normal(u, v));
}

Vec3 Trefoil::normal(f32 u, f32 v) const {
	return cross(tangentU(u, v), tangentV(u, v)).normalized();
}

ChristoffelSymbols Trefoil::christoffelSymbols(f32 u, f32 v) const {
	Vec3 p_xi[] { tangentU(u, v), tangentV(u, v) };

	Vec3 p_x01 = xUv(u, v);
	Vec3 p_x00 = xUu(u, v);
	Vec3 p_x11 = xVv(u, v);
	Vec3 p_xij[2][2]{
		{ p_x00, p_x01 },
		{ p_x01, p_x11 }
	};
	const auto metricTensor = ::firstFundamentalForm(p_xi[0], p_xi[1]);
	const auto metricTensorInverse = metricTensor.inversed();

	Mat2 result[2]{ Mat2::zero, Mat2::zero };
	for (i32 i = 0; i < 2; i++) {
		for (i32 j = 0; j < 2; j++) {
			for (i32 k = 0; k < 2; k++) {
				for (i32 m = 0; m < 2; m++) {
					result[i](j, k) = metricTensorInverse(i, m) * dot(p_xij[j][k], p_xi[m]);
				}
			}
		}
	}
	return {
		.x = result[0],
		.y = result[1]
	};
}

f32 Trefoil::curvature(f32 u, f32 v) const {
	return gaussianCurvature(firstFundamentalForm(u, v), secondFundamentalForm(u, v));
}
