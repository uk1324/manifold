#include "RectParametrization.hpp"
#include "../MatrixMath.hpp"

Mat2 firstFundamentalForm(Vec3 xU, Vec3 xV) {
	const Vec3 p_xi[] = { xU, xV };

	Mat2 metricTensor = Mat2::zero;
	matrixMultiply(
		[&p_xi](i32 i, i32 j) { return p_xi[j].data()[i]; }, 3, 2,
		[&p_xi](i32 i, i32 j) { return p_xi[i].data()[j]; }, 2, 3,
		metricTensor
	);
	return metricTensor;
}

Mat2 secondFundamentalForm(Vec3 xUu, Vec3 xUv, Vec3 xVv, Vec3 normalizedNormal) {
	const auto xUun = dot(xUu, normalizedNormal);
	const auto xVvn = dot(xVv, normalizedNormal);
	const auto xUvn = dot(xUv, normalizedNormal);
	return Mat2(Vec2(xUun, xUvn), Vec2(xUvn, xVvn));
}

f32 gaussianCurvature(const Mat2& firstFundamentalForm, const Mat2& secondFundamentalForm) {
	return secondFundamentalForm.det() / firstFundamentalForm.det();
}

PrincipalCurvatures principalCurvatues(const Mat2& firstFundamentalForm, const Mat2& secondFundamentalForm) {
	// If x_u, x_v are linearly independent the the first fundamental form is invertible.
	const auto shapeOperator = secondFundamentalForm * firstFundamentalForm.inversed();
	const auto a = shapeOperator(0, 0);
	const auto b = shapeOperator(1, 0);
	const auto d = shapeOperator(1, 1);
	// https://www.soest.hawaii.edu/martel/Courses/GG303/Eigenvectors.pdf
	const auto x0 = (a - d);
	const auto delta = sqrt(x0 * x0 + 4.0f * b * b);
	const auto e0 = ((a + d) + delta) / 2.0f;
	const auto e1 = ((a + d) - delta) / 2.0f;
	const auto v0 = Vec2(
		b / sqrt(b * b - pow(e0 - d, 2.0f)), 
		b / sqrt(b * b - pow(e0 - a, 2.0f))
	).normalized();
	const auto v1 = Vec2(-v0.y, v0.x);
	// Alternatively could use the formulas from 
	// https://en.wikipedia.org/wiki/Differential_geometry_of_surfaces#First_and_second_fundamental_forms,_the_shape_operator,_and_the_curvature
	return PrincipalCurvatures{
		.curvature = { e0, e1 },
		.direction = { v0, v1 }
	};
}
