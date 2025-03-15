#pragma once

#include "RectParametrization.hpp"

struct FundamentalForms {
	Mat2 first;
	Mat2 second;
};

template<typename T>
struct GenerateParametrization {
	Vec3 tangentU(f32 u, f32 v) const;
	Vec3 tangentV(f32 u, f32 v) const;
	Vec3 xUu(f32 u, f32 v) const;
	Vec3 xVv(f32 u, f32 v) const;
	Vec3 xUv(f32 u, f32 v) const;
	Vec3 normal(f32 u, f32 v) const;
	ChristoffelSymbols christoffelSymbols(f32 u, f32 v) const;
	f32 curvature(f32 u, f32 v) const;
	PrincipalCurvatures principalCurvatures(f32 u, f32 v) const;
	Mat2 firstFundamentalForm(f32 u, f32 v) const;
	Mat2 secondFundamentalForm(f32 u, f32 v) const;
	FundamentalForms fundamentalForms(f32 u, f32 v) const;
};