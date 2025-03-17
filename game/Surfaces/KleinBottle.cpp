#include "KleinBottle.hpp"

Vec3 KleinBottle::position(f32 u, f32 v) const {
	/*const auto r = 4.0f;
	const auto a = (r + cos(u / 2.0f) * sin(v) - sin(u / 2.0f) * sin(2.0f * v));
	return Vec3(
		a * cos(u),
		a * sin(u),
		sin(u / 2.0f) * sin(v) + cos(u / 2.0f) * sin(2.0f * v)
	) * 0.5f*/;

	//f32 x = 0.0f;
	//f32 y = 0.0f;
	//f32 z = 0.0f;
	//if (v < PI<f32>) {
	//	x = (2.5 - 1.5 * cos(v)) * cos(u);
	//	y = (2.5 - 1.5 * cos(v)) * sin(u);
	//	z = -2.5 * sin(v);
	//} else if (v < TAU<f32>) {
	//	x = (2.5 - 1.5 * cos(v)) * cos(u);
	//	y = (2.5 - 1.5 * cos(v)) * sin(u);
	//	z = 3 * v - 3 * PI<f32>;
	//} else if (v < 3.0f * TAU<f32>) {
	//	x = -2 + (2 + cos(u)) * cos(v);
	//	y = sin(u);
	//	z = (2 + cos(u)) * sin(v) + 3 * PI<f32>;
	//} else {
	//	x = -2 + 2 * cos(v) - cos(u);
	//	y = sin(u);
	//	z = -3 * v + 12 * PI<f32>;
	//}
	//return Vec3(x, y, -z);

	//x(u, v) = v < pi ? (2.5 - 1.5 * cos(v)) * cos(u) : \
	//v < 2 * pi ? (2.5 - 1.5 * cos(v)) * cos(u) : \
	//v < 3 * pi ? -2 + (2 + cos(u)) * cos(v) : -2 + 2 * cos(v) - cos(u)
	//y(u, v) = v < pi ? (2.5 - 1.5 * cos(v)) * sin(u) : \
	//v < 2 * pi ? (2.5 - 1.5 * cos(v)) * sin(u) : \
	//v < 3 * pi ? sin(u) : sin(u)
	//z(u, v) = v < pi ? -2.5 * sin(v) : v < 2 * pi ? 3 * v - 3 * pi : \
	//v < 3 * pi ? (2 + cos(u)) * sin(v) + 3 * pi : -3 * v + 12 * pi


	/*u = angleToRangeZeroTau(u);
	v = angleToRangeZeroTau(v);*/

	//f32 x = 0.0f;
	//f32 y = 0.0f;
	//f32 z = 0.0f;
	//const auto a = 4.0f * (1.0f - 0.5f * cos(u));
	//if (u < PI<f32>) {
	//	x = 6.0f * cos(u) * (1.0f + sin(u)) + a * cos(u) * cos(v);
	//	y = 16.0f * sin(u) + a * sin(u) * cos(v);
	//} else {
	//	x = 6.0f * cos(u) * (1.0f + sin(u)) + a * cos(v + PI<f32>);
	//	y = 16.0f * sin(u);
	//}
	//z = a * sin(v);
	//return Vec3(x, y, z) / 5.0f;

	//return Vec3(

	//)


//	f32 x = 0.0f;
//	f32 y = 0.0f;
//	f32 z = 0.0f;
//	/*x(u,v)= v<2*pi ? (2.5-1.5*cos(v))*cos(u) : \
//        v<3*pi ? -2+(2+cos(u))*cos(v)    : \
//                 -2+2*cos(v)-cos(u)
//y(u,v)= v<2*pi ? (2.5-1.5*cos(v))*sin(u) : \
//                sin(u)
//z(u,v)= v<pi   ? -2.5*sin(v)             : \
//        v<2*pi ? 3*v-3*pi                : \
//        v<3*pi ? (2+cos(u))*sin(v)+3*pi  : \*/
//	std::swap(u, v);
//	if (v < TAU<f32>) {
//		x = (2.5 - 1.5 * cos(v)) * cos(u);
//	} else if (v < 3.0f * PI<f32>) {
//		x = -2 + (2 + cos(u)) * cos(v);
//	} else {
//		x = -2 + 2 * cos(v) - cos(u);
//	}
//
//	if (v < TAU<f32>) {
//		y = (2.5 - 1.5 * cos(v)) * sin(u);
//	} else {
//		y = sin(u);
//	}
//
//	if (v < PI<f32>) {
//		z = -2.5 * sin(v);
//	} else if (v < TAU<f32>) {
//		z = 3 * v - 3 * PI<f32>;
//	} else if (v < 3.0f * PI<f32>) {
//		z = (2 + cos(u)) * sin(v) + 3 * PI<f32>;
//	} else {
//		z = -3 * v + 12 * PI<f32>;
//	}
//	return Vec3(x, y, -z) / 5.0f;
// 	   // The Klein bottle in its classical shape: a further step towards a good parametrization
	// https://arxiv.org/pdf/0909.5354



	// The piecewise parametrizations all have a discontinous change in the direction of motion, probably, because the higher derivatives are discontnious. Or maybe I am doing something wrong with the changing of directions, but the method seems to work with the non piecewise version so it's probably correct.

	// https://pgfplots.net/klein-bottle/

	//return Vec3 (
	//	-2.0f/15.0f * cos(u) * (3*cos(v) - 30*sin(u) + 90 *pow(cos(u), 4.0f) * sin(u) - 60 *pow(cos(u), 6.0f) * sin(u) + 5 * cos(u)*cos(v) * sin(u)),
	//	-1.0f/15.0f * sin(u) * (3*cos(v) 
	//	- 3*pow(cos(u), 2.0f) * cos(v) 
	//	- 48 * pow(cos(u), 4.0f)*cos(v) 
	//	+ 48*pow(cos(u), 6.0f) *cos(v) 
	//	- 60 *sin(u) 
	//	+ 5*cos(u)*cos(v)*sin(u) 
	//	- 5*pow(cos(u),3.0f) * cos(v) *sin(u) 
	//	- 80*pow(cos(u), 5.0f) * cos(v)*sin(u) 
	//	+ 80*pow(cos(u), 7.0f) * cos(v) * sin(u)),
	//	2.0f/15.0f * (3 + 5*cos(u) *sin(u))*sin(v)
	//);

	//const auto a = 20.0f;
	//const auto b = 8.0f;
	//const auto c = 11.0f / 2.0f;
	//const auto d = 2.0f / 5.0f;
	//const auto t = u;
	//const auto curve = Vec2(
	//	a * (1.0f - cos(t)),
	//	b * sin(t) * (1.0f - cos(t))
	//);
	//const auto r = c - d * (t - PI<f32>) * sqrt(std::max(0.0f, t * (TAU<f32> -t)));
	//const auto tangent = Vec2(a * sin(t), b * pow(sin(t), 2.0f) - b * pow(cos(t), 2.0f) + b * cos(t)).normalized();
	//const auto normal = Vec2(-tangent.y, tangent.x);
	//const auto binormal = Vec3(0.0f, 0.0f, 1.0f);
	//const auto result = Vec3(curve.x, curve.y, 0.0f)
	//	+ r * (cos(v) * Vec3(normal.x, normal.y, 0.0f))
	//	+ r * (sin(v) * binormal);
	//return result / 15.0f;

	//const auto t = u;
	//const auto curve = Vec2(
	//	5.0f * sin(t),
	//	2.0f * pow(sin(t), 2.0f) * cos(t)
	//);
	//const auto r = 0.5f - (1.0f / 30.0f) * (2.0f * t - PI<f32>) * sqrt(abs(2.0f * t * (TAU<f32> -2.0f * t)));
	//const auto tangent = Vec2(
	//	5.0f * cos(t), 
	//	4.0f * pow(cos(t), 2.0f) * sin(t) - 2.0f * pow(sin(t), 3.0f)
	//).normalized();
	//const auto normal = Vec2(-tangent.y, tangent.x);
	//const auto binormal = Vec3(0.0f, 0.0f, 1.0f);
	//const auto result = Vec3(curve.x, curve.y, 0.0f)
	//	+ r * (cos(v) * Vec3(normal.x, normal.y, 0.0f))
	//	+ r * (sin(v) * binormal);
	//return result;

	// https://en.wikipedia.org/wiki/Talk%3AKlein_bottle
	// Response by tamfang
	const auto k = 0.15f;
	const auto r = k * (2 + sin(2.0f * u) + sin(4.0f * u) / 2.0f);
	const auto x = sin(2.0f * u) / 3.0f - sin(4.0f * u) / 5.0f + r * cos(v) * cos(u - sin(2.0f * u));
	const auto y = cos(2.0f * u) - r * cos(v) * sin(u - sin(2.0f * u));
	const auto z = r * sin(v);
	return Vec3(x, y, z) * 2.0f;
}