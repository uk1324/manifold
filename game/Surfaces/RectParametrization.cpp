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
