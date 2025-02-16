#include "Trefoil.hpp"
#include <engine/Math/Derivative.hpp>

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

Vec3 Trefoil::normal(f32 u, f32 v) const {
	return cross(tangentU(u, v), tangentV(u, v)).normalized();
}

template<typename Matrix1, typename Matrix2, typename MatrixOutput>
void matrixMultiply(Matrix1 a, i32 aSizeX, i32 aSizeY, Matrix2 b, i32 bSizeX, i32 bSizeY, MatrixOutput& output) {
	// Matrix multiplicaiton just multiplies each column of rhs by lhs, because of this the output column count is the same as rhs column count.
	// The output dimension of the matrix is it's height so the height of the output is the height of the lhs.
	ASSERT(aSizeX == bSizeY);
	const auto sumIndexMax = aSizeX;
	const auto outputSizeY = aSizeY;
	const auto outputSizeX = bSizeX;

	for (i64 row = 0; row < outputSizeY; row++) {
		for (i64 column = 0; column < outputSizeX; column++) {
			output(column, row) = 0;
			for (i64 i = 0; i < sumIndexMax; i++) {
				output(column, row) += a(i, row) * b(column, i);
			}
		}
	}
}

ChristoffelSymbols Trefoil::christoffelSymbols(f32 u, f32 v) const {
	Vec3 p_xi[] { tangentU(u, v), tangentV(u, v) };

	Vec3 p_x01 = mixedDerivativeMidpoint([this](f32 u, f32 v) { return position(u, v); }, u, v, step, step);
	Vec3 p_x00 = secondDerivativeMidpoint([this, &v](f32 u) { return position(u, v); }, u, step);
	Vec3 p_x11 = secondDerivativeMidpoint([this, &u](f32 v) { return position(u, v); }, v, step);
	Vec3 p_xij[2][2]{
		{ p_x00, p_x01 },
		{ p_x01, p_x11 }
	};
	Mat2 metricTensor = Mat2::zero;
	matrixMultiply(
		[&p_xi](i32 i, i32 j) { return p_xi[j].data()[i]; }, 3, 2,
		[&p_xi](i32 i, i32 j) { return p_xi[i].data()[j]; }, 2, 3,
		metricTensor
	);
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
