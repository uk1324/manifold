#include "GenerateParametrization.hpp"

// x(u, v) = (f(u, v), g(u, v), h(u, v))
struct DerivativesUpToSeondOrder2To3 {
	Vec3 xU;
	Vec3 xV;
	Vec3 xUu;
	Vec3 xVv;
	Vec3 xUv;
};

template<typename Function, typename Scalar>
DerivativesUpToSeondOrder2To3 midpointDerivativesUpToSeondOrder2To3(Function f, Scalar u, Scalar v, Scalar h) {
	const auto f_u_v = f(u, v);
	const auto f_uph_v = f(u + h, v);
	const auto f_umh_v = f(u - h, v);
	const auto f_u_vph = f(u, v + h);
	const auto f_u_vmh = f(u, v - h);
	const auto twoH = (Scalar(2) * h);
	const auto h2 = h * h;
	return DerivativesUpToSeondOrder2To3{
		.xU = (f_uph_v - f_umh_v) / twoH,
		.xV = (f_u_vph - f_u_vmh) / twoH,
		.xUu = (f_umh_v - Scalar(2) * f_u_v + f_uph_v) / h2,
		.xVv = (f_u_vmh - Scalar(2) * f_u_v + f_u_vph) / h2,
		.xUv = mixedDerivativeMidpoint(f, u, v, h, h)
	};
}

#define self (*static_cast<T*>(this))
#define midpointDerivatives() midpointDerivativesUpToSeondOrder2To3([this](f32 u, f32 v) { return self.position(u, v); })

template<typename T>
Vec3 GenerateParametrization<T>::tangentU(f32 u, f32 v) const {
	return approximateTangentU(self, u, v);
}

template<typename T>
Vec3 GenerateParametrization<T>::tangentV(f32 u, f32 v) const {
	return approximateTangentV(self, u, v);
}

template<typename T>
Vec3 GenerateParametrization<T>::xUu(f32 u, f32 v) const {
	return approximateXuu(self, u, v);
}

template<typename T>
Vec3 GenerateParametrization<T>::xVv(f32 u, f32 v) const {
	return approximateXvv(self, u, v);
}

template<typename T>
Vec3 GenerateParametrization<T>::xUv(f32 u, f32 v) const {
	return approximateXuv(self, u, v);
}

template<typename T>
Vec3 GenerateParametrization<T>::normal(f32 u, f32 v) const {
	return surfaceNormal(tangentU(u, v), tangentV(u, v));
}

template<typename T>
ChristoffelSymbols GenerateParametrization<T>::christoffelSymbols(f32 u, f32 v) const {
	const auto d = midpointDerivatives();
	return ::christoffelSymbols(d.xU, d.xV, d.xUu, d.xUv, d.xVv);
}

template<typename T>
f32 GenerateParametrization<T>::curvature(f32 u, f32 v) const {
	const auto f = fundamentalForms(u, v);
	return gaussianCurvature(f.first, f.second);
}

template<typename T>
PrincipalCurvatures GenerateParametrization<T>::principalCurvatures(f32 u, f32 v) const {
	const auto f = fundamentalForms(u, v);
	return principalCurvatues(f.first, f.second);
}

template<typename T>
Mat2 GenerateParametrization<T>::firstFundamentalForm(f32 u, f32 v) const {
	return ::firstFundamentalForm(tangentU(u, v), tangentV(u, v));
}

template<typename T>
Mat2 GenerateParametrization<T>::secondFundamentalForm(f32 u, f32 v) const {
	const auto d = midpointDerivatives();
	// The surfaces normal needs to calculate xU, xV so reuse the values from calculating the second order derivatives.
	const auto normal = surfaceNormal(d.xU, d.xV);
	return ::secondFundamentalForm(d.xUu, d.xUv, d.xVv, normal);
}

template<typename T>
FundamentalForms GenerateParametrization<T>::fundamentalForms(f32 u, f32 v) const {
	const auto d = midpointDerivatives();
	const auto normal = surfaceNormal(d.xU, d.xV);
	return FundamentalForms{
		.first = ::firstFundamentalForm(d.xU, d.xV),
		.second = ::secondFundamentalForm(d.xUu, d.xUv, d.xVv, normal)
	};
}
