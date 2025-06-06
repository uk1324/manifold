#include "ProjectivePlane.hpp"
#include <complex>

Vec3 ProjectivePlane::position(f32 u, f32 v) const {
	using C = std::complex<f32>;
	//const auto w = C(cos(v), sin(v)) * u;
	const auto w = u * exp(C(0.0f, v));
	const auto w2 = w * w;
	const auto w4 = w2 * w2;
	const auto w3 = w2 * w;
	const auto w6 = w3 * w3;
	const auto denom = w6 + sqrt(5.0f) * w3 - 1.0f;
	const auto g1 = -(3.0f / 2.0f) * (w * (1.0f - w4) / denom).imag();
	const auto g2 = -(3.0f / 2.0f) * (w * (1.0f + w4) / denom).real();
	const auto g3 = ((1.0f + w6) / denom).imag() - 0.5f;
	const auto l = g1 * g1 + g2 * g2 + g3 * g3;
	return Vec3(g1, g2, g3) / l;
}
