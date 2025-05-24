#include "Stereographic.hpp"
#include <engine/Math/Angles.hpp>
#include <engine/Math/Quat.hpp>
#include <engine/Math/Plane.hpp>
#include <engine/Math/Circle.hpp>

bool isPointAtInfinity(Vec3 v) {
	return isinf(v.x);
}

Vec3 stereographicProjection(Vec4 p) {
	// https://en.wikipedia.org/wiki/Stereographic_projection
	const auto a = 1.0f / (1.0f - p.w);
	if (isnan(a) || isinf(a)) {
		return Vec3(INFINITY, INFINITY, INFINITY);
	}
	// p.x, p.y, p.z are all <= 1 so they shouldn't overlflow when multiplied by a finite constant a.
	return Vec3(
		p.x * a,
		p.y * a,
		p.z * a
	);
}

/*
f(x, y, z, w) -> (x / (1 - w), y / (1 - w), z / (1 - w))

The jacobian is
[ f1_x, f1_y, f1_z, f1_w ]
[ f2_x, f2_y, f2_z, f2_w ]
[ f3_x, f3_y, f3_z, f3_w ]

[ 1/(1 - w) 0         0         x/(1-w)^2 ]
[ 0         1/(1 - w) 0         y/(1-w)^2 ]
[ 0         0         1/(1 - w) z/(1-w)^2 ]
*/
Vec3 stereographicProjectionJacobian(Vec4 atPoint, Vec4 onVector) {
	const auto s = 1.0f / (1.0f - atPoint.w);
	const auto s2 = s * s;
	return Vec3(
		s * onVector.x + atPoint.x * s2 * onVector.w,
		s * onVector.y + atPoint.y * s2 * onVector.w,
		s * onVector.z + atPoint.z * s2 * onVector.w
	);
}

Vec4 inverseStereographicProjection(Vec3 p) {
	if (isPointAtInfinity(p)) {
		return Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	const auto s = p.x * p.x + p.y * p.y + p.z * p.z;
	const auto a = 2.0f / (s + 1.0f);
	return Vec4(
		p.x * a,
		p.y * a,
		p.z * a,
		(s - 1.0f) / (s + 1.0f)
	);
}

/*
[ f1_x, f1_y, f1_z ]
[ f2_x, f2_y, f2_z ]
[ f3_x, f3_y, f3_z ]
[ f4_x, f4_y, f4_z ]
*/
Vec4 inverseStereographicProjectionJacobian(Vec3 at, Vec3 of) {
	const auto x2 = at.x * at.x;
	const auto y2 = at.y * at.y;
	const auto z2 = at.z * at.z;

	const auto s = x2 + y2 + z2 + 1.0f;
	const auto s1 = 2.0f / (s * s);

	const auto m00 = (s - 2.0f * x2) * s1;
	const auto m11 = (s - 2.0f * y2) * s1;
	const auto m22 = (s - 2.0f * z2) * s1;
	const auto m10 = (-2.0f * at.x * at.y * s1);
	const auto& m01 = m10;
	const auto m20 = (-2.0f * at.x * at.z * s1);
	const auto& m02 = m20;
	const auto m21 = (-2.0f * at.y * at.z * s1);
	const auto& m12 = m21;
	const auto m03 = at.x * s1;
	const auto m13 = at.y * s1;
	const auto m23 = at.z * s1;

	return Vec4(
		m00 * of.x + m10 * of.y + m20 * of.z,
		m01 * of.x + m11 * of.y + m21 * of.z,
		m02 * of.x + m12 * of.y + m22 * of.z,
		m03 * of.x + m13 * of.y + m23 * of.z
	);
}

/*
The geodesics are parts of great circles passing though S3. If you have a point p then the point -p also passes though it.
Any great circle can be written as the intersection dot(p, p) = 1 and dot(n, p) = 0.
dot(-p, -p) = (-1)^2 dot(p, p) = 1
and
dot(n, -p) = 0 = -dot(n, p) = 0
*/
Vec3 antipodalPoint(Vec3 p) {
	if (isPointAtInfinity(p)) {
		return Vec3(0.0f);
	}
	return stereographicProjection(-inverseStereographicProjection(p));
}

std::optional<f32> rayStereographicPlaneIntersection(Vec3 rayOrigin, Vec3 rayDirection, const StereographicPlane& plane) {
	if (plane.type == StereographicPlane::Type::PLANE) {
		return rayPlaneIntersection(rayOrigin, rayDirection, plane.plane);
	} else {
		return raySphereIntersection(rayOrigin, rayDirection, plane.sphere);
	}
}

std::optional<f32> rayStereographicPlaneIntersection(const Ray3& ray, const StereographicPlane& plane) {
	return rayStereographicPlaneIntersection(ray.origin, ray.origin, plane);
}

std::optional<f32> rayStereographicPolygonIntersection(Vec3 rayOrigin, Vec3 rayDirection, const StereographicPlane& plane, View<const Vec4> edgeNormals) {
	const auto t = rayStereographicPlaneIntersection(rayOrigin, rayDirection, plane);
	if (!t.has_value()) {
		return std::nullopt;
	}
	const auto& hitP = rayAt(*t, rayOrigin, rayDirection);
	const auto hitP4 = inverseStereographicProjection(hitP);
	for (const auto& normal : edgeNormals) {
		if (dot(hitP4, normal) < 0.0f) {
			return std::nullopt;
		}
	}
	return t;
}

std::optional<f32> rayStereographicPolygonIntersection(const Ray3& ray, const StereographicPlane& plane, View<const Vec4> edgeNormals) {
	return rayStereographicPolygonIntersection(ray.origin, ray.direction, plane, edgeNormals);
}

Vec3 moveForwardStereographic(Vec3 initialPosition, Vec3 velocityDirection, f32 velocityLength) {
	const auto p = inverseStereographicProjection(initialPosition);
	const auto pq = Quat(p.x, p.y, p.z, p.w).normalized();
	const auto v = inverseStereographicProjectionJacobian(initialPosition, velocityDirection);
	Quat vq(v.x, v.y, v.z, v.w);
	vq = vq.normalized();
	vq *= velocityLength;
	vq *= pq.inverseIfNormalized();
	Quat movement = quatExp(Vec3(vq.x, vq.y, vq.z));
	Quat point = (pq * movement).normalized();
	return stereographicProjection(Vec4(point.x, point.y, point.z, point.w));
}

Sphere stereographicSphere(Vec4 center, f32 radius) {
	const auto p = stereographicProjection(center);
	const auto p0 = moveForwardStereographic(p, Vec3(1.0f, 0.0f, 0.0f), radius);
	const auto p1 = moveForwardStereographic(p, Vec3(0.0f, 1.0f, 0.0f), radius);
	const auto p2 = moveForwardStereographic(p, Vec3(0.0f, 0.0f, 1.0f), radius);
	const auto p3 = moveForwardStereographic(p, Vec3(-1.0f, 0.0f, 0.0f), radius);
	const auto s = Sphere::thoughPoints(p0, p1, p2, p3);
	return s;
}

CircularSegment::CircularSegment(Vec3 start, Vec3 initialVelocity, Vec3 center, f32 angle)
	: start(start)
	, initialVelocity(initialVelocity)
	, center(center)
	, angle(angle) {}

Vec3 CircularSegment::sample(f32 angle) const {
	return center + cos(angle) * start + sin(angle) * initialVelocity;
}

LineSegment::LineSegment(Vec3 e0, Vec3 e1)
	: e{ e0, e1 } {}


StereographicSegment::StereographicSegment(Vec3 start, Vec3 initialVelocity, Vec3 center, f32 angle)
	: circular(start, initialVelocity, center, angle)
	, type(Type::CIRCULAR) {}

StereographicSegment::StereographicSegment(Vec3 e0, Vec3 e1)
	: line(e0, e1)
	, type(Type::LINE) {}

StereographicSegment StereographicSegment::fromCircular(Vec3 start, Vec3 initialVelocity, Vec3 center, f32 angle) {
	return StereographicSegment(start, initialVelocity, center, angle);
}

StereographicSegment StereographicSegment::fromLine(Vec3 e0, Vec3 e1) {
	return StereographicSegment(e0, e1);
}

StereographicSegment StereographicSegment::fromEndpoints(Vec4 e0, Vec4 e1) {
	const auto p0 = stereographicProjection(e0);
	const auto p1 = stereographicProjection(e1);
	if (isPointAtInfinity(p0) && isPointAtInfinity(p1)) {
		return StereographicSegment::fromLine(p0, p1);
	}
	if (isPointAtInfinity(p0) || isPointAtInfinity(p1)) {
		auto atInfinity = p0;
		auto finite = p1;
		if (isPointAtInfinity(finite)) {
			std::swap(atInfinity, finite);
		}
		const auto direction = finite.normalized();

	} else {
		auto p2 = antipodalPoint(p0);
		if (isPointAtInfinity(p2)) {
			p2 = antipodalPoint(p1);
			if (isPointAtInfinity(p2)) {
				ASSERT_NOT_REACHED();
				return StereographicSegment::fromLine(p0, p1);
			}
		}

		const auto velocity4 = (e1 - dot(e1, e0.normalized()) * e0.normalized()).normalized();
		const auto velocity3 = stereographicProjectionJacobian(e0, velocity4);

		const auto origin = p0;
		Vec3 v0 = p2 - origin;
		Vec3 v1 = p1 - origin;
		const auto b0 = v0.normalized();
		const auto b1 = (v1 - dot(v1, b0) * b0).normalized();
		const auto t0 = dot(b0, b1);
		auto coordinatesInBasis = [&b0, &b1](Vec3 v) -> Vec2 {
			return Vec2(dot(v, b0), dot(v, b1));
			};
		auto fromCoordinatesInBasis = [&b0, &b1, &origin](Vec2 v) -> Vec3 {
			return v.x * b0 + v.y * b1 + origin;
			};
		const auto c0 = Vec2(0.0f);
		const auto c1 = coordinatesInBasis(v0);
		const auto c2 = coordinatesInBasis(v1);
		const auto circle = Circle::thoughPoints(c0, c1, c2);
		Vec3 center = fromCoordinatesInBasis(circle.center);
		const auto radius = circle.radius;

		//const auto midpoint = ((p2 - center) + (p1 - center)).normalized() + center;

		auto circularDistance = [](Vec3 a, Vec3 b) {
			return acos(std::clamp(dot(a.normalized(), b.normalized()), -1.0f, 1.0f));
			};
		/*
		Wanted to try calculating the arclength by calculating it normally between p0, p1 then checking the the endpoint is correct (by checking the distance of the evavluated curve endpoint to the correct endpoint) and
		*/

		/*i32 stepCount = 10;
		const auto arclength4 = acos(std::clamp(dot(e0, e1), -1.0f, 1.0f));
		auto previous = e0;
		f32 d = 0.0f;
		for (i32 i = 1; i < stepCount; i++) {
			const auto t = f32(i) / f32(stepCount - 1);
			const auto a = t * arclength4;
			const auto current = cos(a) * e0 + sin(a) * velocity4;
			d += circularDistance(
				stereographicProjection(previous) - center,
				stereographicProjection(current) - center);
			previous = current;
		}*/
		f32 d = circularDistance(p0 - center, p1 - center);
		const auto p = (p0 - center);
		const auto v = velocity3.normalized() * p.length();
		{
			const auto calculatedEndpoint = center + p * cos(d) + v * sin(d);
			//renderer.sphere(calculatedEndpoint, width * 3, Color3::CYAN);
			const auto correctEndpoint = p1;
			if (calculatedEndpoint.distanceTo(correctEndpoint) > 0.01f) {
				d = TAU<f32> -d;
			}
		}
		// This isn't actually the midpoint so there can the correct between this and p0 and p1 might not be the shortest.

		//const auto midpoint = stereographicProjection((e0 + e1).normalized());
		// 
		/*const auto d =
			circularDistance(p0 - center, midpoint - center) +
			circularDistance(p1 - center, midpoint - center);*/

			//lineGenerator.addCircularArc(p0 - center, velocity3, center, d, width);
		return StereographicSegment::fromCircular(p, v, center, d);
		//lineGenerator.addCircularArc(p0 - center, velocity3, center, TAU<f32>, width);
		const auto t1 = p0.distanceTo(center);
		const auto t2 = p1.distanceTo(center);
		//const auto t3 = midpoint.distanceTo(center);
		/*renderer.sphere(p0, width * 3, Color3::RED);
		renderer.sphere(p1, width * 3, Color3::RED);
		renderer.sphere(midpoint, width * 3, Color3::BLUE);
		renderer.sphere(center, width * 3, Color3::GREEN);
		renderer.line(p0, p0 + velocity3.normalized() * 0.5f, 0.01f, Color3::MAGENTA);*/
		//renderer.line(p0, p1, width, Color3::WHITE);
		/*renderer.sphere(p0, width * 3, Color3::RED);
		renderer.sphere(p1, width * 3, Color3::RED);*/
		//renderer.sphere(midpoint, width * 3, Color3::BLUE);
	}
}

StereographicPlane StereographicPlane::fromVertices(Vec4 v0, Vec4 v1, Vec4 v2) {
	const auto p4 = -v0;
	const auto sp0 = stereographicProjection(v0);
	const auto sp1 = stereographicProjection(v1);
	const auto sp2 = stereographicProjection(v2);
	const auto sp3 = stereographicProjection(p4);

	const auto planeThoughPolygonVertices = Plane::fromPoints(sp0, sp1, sp2);
	auto deviationFromPlane = [&planeThoughPolygonVertices](Vec4 e0, Vec4 e1) -> f32 {
		const auto s = StereographicSegment::fromEndpoints(e0, e1);
		switch (s.type) {
			using enum StereographicSegment::Type;
		case LINE:
			return 0.0f;
		case CIRCULAR:
			const auto circularMid = s.circular.sample(s.circular.angle / 2.0f);
			return planeThoughPolygonVertices.distance(circularMid);
		}
	};
	const auto deviation = std::max(
		deviationFromPlane(v0, v1),
		std::max(
			deviationFromPlane(v1, v2), deviationFromPlane(v2, v0)
		)
	);
	static float maxAllowedDeviation = 0.005f;
	//ImGui::SliderFloat("max allowed deviation", &maxAllowedDeviation, 0.0, 0.03f);

	// Could check if the deviation is less than the radius of the tubes.
	if (deviation < maxAllowedDeviation) {
		return StereographicPlane{
			.type = StereographicPlane::Type::PLANE, 
			.plane = planeThoughPolygonVertices
		};
	} else {
		return StereographicPlane{
			.type = StereographicPlane::Type::SPHERE,
			.sphere = Sphere::thoughPoints(sp0, sp1, sp2, sp3)
		};
	}
}
