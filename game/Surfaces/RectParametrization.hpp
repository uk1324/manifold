#pragma once

#include <engine/Math/Derivative.hpp>
#include <engine/Math/Vec3.hpp>
#include <engine/Math/Mat2.hpp>

struct ChristoffelSymbols {
	Mat2 x;
	Mat2 y;
};

enum class SquareSideConnectivity {
	NONE,
	NORMAL,
	REVERSED
};

// The uv tangents of the surface can also be called partial velocities.

Vec3 surfaceNormal(Vec3 tangentU, Vec3 tangentV);

const auto step = 0.05f;

// TODO: Make a function that calculates all the first and second order derivatives at once.
template<typename Surface>
Vec3 approximateTangentU(const Surface& s, f32 u, f32 v) {
	return derivativeMidpoint([&v, &s](f32 u) { return s.position(u, v); }, u, step);
}

template<typename Surface>
Vec3 approximateTangentV(const Surface& s, f32 u, f32 v) {
	return derivativeMidpoint([&u, &s](f32 v) { return s.position(u, v); }, v, step);
}

template<typename Surface>
Vec3 approximateXuu(const Surface& s, f32 u, f32 v) {
	return secondDerivativeMidpoint([&s, &v](f32 u) { return s.position(u, v); }, u, step);
}

template<typename Surface>
Vec3 approximateXvv(const Surface& s, f32 u, f32 v) {
	return secondDerivativeMidpoint([&s, &u](f32 v) { return s.position(u, v); }, v, step);
}

template<typename Surface>
Vec3 approximateXuv(const Surface& s, f32 u, f32 v) {
	return mixedDerivativeMidpoint([&s](f32 u, f32 v) { return s.position(u, v); }, u, v, step, step);
}

Mat2 firstFundamentalForm(Vec3 xU, Vec3 xV);
Mat2 secondFundamentalForm(Vec3 xUu, Vec3 xUv, Vec3 xVv, Vec3 normalizedNormal);
f32 gaussianCurvature(const Mat2& firstFundamentalForm, const Mat2& secondFundamentalForm);
ChristoffelSymbols christoffelSymbols(Vec3 xU, Vec3 xV, Vec3 xUu, Vec3 xUv, Vec3 xVv);

struct PrincipalCurvatures {
	PrincipalCurvatures(f32 c0, Vec2 v0, f32 c1, Vec2 v1);
	f32 curvature[2];
	Vec2 direction[2];
};
PrincipalCurvatures principalCurvatues(const Mat2& firstFundamentalForm, const Mat2& secondFundamentalForm);

/*

https://faculty.sites.iastate.edu/jia/files/inline-files/surface-curves.pdf
https://faculty.sites.iastate.edu/jia/foundations-robotics-and-computer-vision-coms-47705770
https://faculty.sites.iastate.edu/jia/files/inline-files/surface-curvature.pdf
To measure the curvature of a surface at a point the curvature of the curves lying on the surfaces going through this point is analyzed.

If we have some unit speed curve a(t) then we can create a basis using the surface normal N and the velocity of the curve T = a'(t). Let B = NxT.
Then we can express the acceleration of the curve in this basis.
By differentating <a', a'> = 1 we get that <T, a''> = 0.
So a'' = k_n N + k_g B.
k_n is the normal curvature and k_g is the geodesic curvature.
Because N, B are orthogonal we have that k_n = <a'', N>.
Representing a' = (u', v') in the tangent space basis we get that.
a'' =
<(x_u u' + x_v v')', N> =
<N, x_u u'' + (x_uu u' + x_uv v')u' + (x_uv u' + x_vv v')v'> =
The x_u and x_v terms cancel, because they are parallel to the normal
<x_uu, N> u'^2 + 2<x_uv, N>u'v' + <x_vv, N> v'^2
The matrix
II =
[<x_uu, N> <x_uv, N>]
[<x_uu, N> <x_vv, N>]
is the second fundamental form and it allows expressing the normal curvature of a curve going in a given direction.

If the vector w is not unit length then it can be expressed as |w| * m.
Then II(w) = |w|^2 II(m)  = I(w) II(m) = I(w) * k_n
//Explicit written out it says
//<w, II w> = k_n <w, I w>
//<w, (II - k_n I) w> = 0
//It doesn't make sense for w to be 0, so w != 0, but then this implies that (II - k_n I) = 0, because
k_n = II(w) / I(w)

Not sure how to continue the derivation. The general idea is the the solution to minizming the curvature can be obtained by solving the eigenvalue problem II I^-1 w = k_n w.

It should be noted that the curvature of the cruve != normal curvature. It is always true that the curvature of the curve k^2 = k_n^2 + k_g^2

The general idea is that the second fundamental form measures the normal curvature in a given unit direction.

*/