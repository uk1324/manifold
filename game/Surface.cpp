#include <game/Surface.hpp>
#include <game/Tri3d.hpp>
//#include <game/MeshUtils.hpp>
#include <game/Constants.hpp>
#include <engine/Math/OdeIntegration/RungeKutta4.hpp>
#include <engine/Math/Vec4.hpp>
#include <game/MeshUtils.hpp>
#include <engine/Math/Interpolation.hpp>

template<typename RectParametrization>
void initializeSurface(
	Surface& surface,
	const RectParametrization& parametrization) {
	//const auto size = 100;
	const auto size = 50;
	const auto sizeU = 4 * size;
	const auto sizeV = size;
	surface.indices.clear();
	surface.positions.clear();
	surface.uvs.clear();
	surface.uvts.clear();
	surface.normals.clear();
	surface.curvatures.clear();

	for (i32 vi = 0; vi <= sizeV; vi++) {
		for (i32 ui = 0; ui <= sizeU; ui++) {
			const auto ut = f32(ui) / sizeU;
			const auto vt = f32(vi) / sizeV;
			const auto u = lerp(parametrization.uMin, parametrization.uMax, ut);
			const auto v = lerp(parametrization.vMin, parametrization.vMax, vt);
			const auto p = parametrization.position(u, v);
			const auto n = parametrization.normal(u, v);

			surface.curvatures.push_back(parametrization.curvature(u, v));
			surface.addVertex(p, n, Vec2(u, v), Vec2(ut, vt));
		}
	}
	{
		const auto r = std::ranges::minmax(surface.curvatures);
		surface.minCurvature = r.min;
		surface.maxCurvature = r.max;
	}


	auto index = [&sizeU](i32 ui, i32 vi) {
		//// Wrap aroud
		//if (ui == size) { ui = 0; }
		//if (vi == size) { vi = 0; }

		return vi * (sizeU + 1) + ui;
		};
	for (i32 vi = 0; vi < sizeV; vi++) {
		for (i32 ui = 0; ui < sizeU; ui++) {
			const auto i0 = index(ui, vi);
			const auto i1 = index(ui + 1, vi);
			const auto i2 = index(ui + 1, vi + 1);
			const auto i3 = index(ui, vi + 1);
			indicesAddQuad(surface.indices, i0, i1, i2, i3);
		}
	}

	surface.triangleCenters.clear();
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		Vec3 triangle[3];
		getTriangle(surface.positions, surface.indices, triangle, i);
		surface.triangleCenters.push_back(triCenter(triangle));
	}

	surface.sortedTriangles.clear();
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		surface.sortedTriangles.push_back(i);
	}

	surface.triangleAreas.clear();
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		Vec3 vs[3];
		getTriangle(surface.positions, surface.indices, vs, i);
		surface.triangleAreas.push_back(triArea(vs));
	}
	f32 totalArea = 0.0f;
	for (i32 i = 0; i < surface.triangleCount(); i++) {
		totalArea += surface.triangleAreas[i];
	}
	surface.totalArea = totalArea;
}

void Surface::initialize(Type selected) {
	this->selected = selected;
	switch (selected) {
	case Surface::Type::TORUS:
		initializeSurface(*this, torus);
		break;
	}
}

SurfaceTangent Surface::scaleTangent(SurfaceTangent tangent, f32 scale) const {
	return SurfaceTangent::makeUv(tangent.uv * scale);
}

Vec3 Surface::position(SurfacePosition pos) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.position(pos.uv.x, pos.uv.y);
	}
	ASSERT_NOT_REACHED();
}

Vec3 Surface::normal(SurfacePosition pos) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.normal(pos.uv.x, pos.uv.y);
	}
	ASSERT_NOT_REACHED();
}

Vec3 Surface::tangentU(SurfacePosition pos) const{
	switch (selected) {
		using enum Type;
	case TORUS: return torus.tangentU(pos.uv.x, pos.uv.y);
	}
	ASSERT_NOT_REACHED();
}

Vec3 Surface::tangentV(SurfacePosition pos) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.tangentV(pos.uv.x, pos.uv.y);
	}
	ASSERT_NOT_REACHED();
}

ChristoffelSymbols Surface::christoffelSymbols(SurfacePosition pos) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.christoffelSymbols(pos.uv.x, pos.uv.y);
	}
	ASSERT_NOT_REACHED();
}

SurfacePosition Surface::randomPointOnSurface() {
	const auto value = uniform01(rng) * totalArea;
	f32 cursor = 0.0f;
	i32 randomTriangleIndex = 0;
	for (i32 i = 0; i < triangleCount(); i++) {
		cursor += triangleAreas[i];
		if (cursor >= value) {
			randomTriangleIndex = i;
			break;
		}
	}
	Vec2 triUvs[3];
	getTriangle(uvs, indices, triUvs, randomTriangleIndex);
	const auto r0 = uniform01(rng);
	const auto r1 = uniform01(rng);
	const auto uv = uniformRandomPointOnTri(triUvs, r0, r1);
	return SurfacePosition::makeUv(uv);
}

SurfaceTangent Surface::randomTangentVectorAt(SurfacePosition position, f32 length) {
	f32 randomAngle = uniform01(rng) * TAU<f32>;
	return tangentVectorFromPolar(position, randomAngle, length);
}

SurfaceTangent Surface::tangentVectorFromPolar(SurfacePosition position, f32 angle, f32 length) {
	const auto uTangent = tangentU(position);
	const auto vTangent = tangentV(position);
	const auto normal = cross(uTangent, vTangent);
	const auto orthonormalTangentSpaceBasis0 = uTangent.normalized();
	const auto orthonormalTangentSpaceBasis1 = cross(uTangent, normal).normalized();
	const auto randomUnitTangentVector =
		(cos(angle) * orthonormalTangentSpaceBasis0 +
		sin(angle) * orthonormalTangentSpaceBasis1) * length;

	const auto randomUnitTangentVectorInUvBasis = vectorInTangentSpaceBasis(
		randomUnitTangentVector, uTangent, vTangent, normal);
	return SurfaceTangent::makeUv(randomUnitTangentVectorInUvBasis);
}

f32 Surface::uMin() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.uMin;
	}
	ASSERT_NOT_REACHED();
}

f32 Surface::uMax() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.uMax;
	}
	ASSERT_NOT_REACHED();
}

f32 Surface::vMin() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.vMin;
	}
	ASSERT_NOT_REACHED();
}

f32 Surface::vMax() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.vMax;
	}
	ASSERT_NOT_REACHED();
}

SquareSideConnectivity Surface::uConnectivity() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.uConnectivity;
	}
	ASSERT_NOT_REACHED();
}

SquareSideConnectivity Surface::vConnectivity() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.vConnectivity;
	}
	ASSERT_NOT_REACHED();
}

void Surface::integrateParticle(SurfacePosition& position, SurfaceTangent& velocity) {
	Vec2 velocityUv = velocity.uv;
	const auto uTangent = tangentU(position);
	const auto vTangent = tangentV(position);

	auto movementRhs = [&](Vec4 state, f32 _) {
		const auto symbols = christoffelSymbols(SurfacePosition::makeUv(Vec2(state.x, state.y)));
		Vec2 velocity(state.z, state.w);

		return Vec4(
			velocity.x,
			velocity.y,
			-dot(velocity, symbols.x * velocity),
			-dot(velocity, symbols.y * velocity)
		);
	};

	const auto a = movementRhs(Vec4(position.uv.x, position.uv.y, velocityUv.x, velocityUv.y), 0.0f);

	i32 n = 5;
	Vec4 state(position.uv.x, position.uv.y, velocityUv.x, velocityUv.y);
	for (i32 i = 0; i < n; i++) {
		state = rungeKutta4Step(movementRhs, state, 0.0f, Constants::dt / n);
	}

	position.uv = Vec2(state.x, state.y);
	velocity.uv = Vec2(state.z, state.w);
}

void Surface::sortTriangles(Vec3 cameraPosition) {
	// @Performance: Could discard triangles behind the camera.
	std::vector<f32> distances;
	for (i32 i = 0; i < triangleCenters.size(); i++) {
		distances.push_back(triangleCenters[i].distanceSquaredTo(cameraPosition));
	}
	const auto lessThan = [&](i32 a, i32 b) {
		return distances[a] > distances[b];
		};
	std::sort(sortedTriangles.begin(), sortedTriangles.end(), lessThan);
}

i32 Surface::vertexCount() const {
	return i32(positions.size());
}

i32 Surface::triangleCount() const {
	return i32(indices.size() / 3);
}

void Surface::addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt) {
	positions.push_back(p);
	normals.push_back(n);
	uvs.push_back(uv);
	uvts.push_back(uvt);
}

SurfacePosition SurfacePosition::makeUv(Vec2 uv) {
	return SurfacePosition(uv);
}

SurfacePosition::SurfacePosition(Vec2 uv)
	: uv(uv) {}


Vec2 vectorInTangentSpaceBasis(Vec3 v, Vec3 tangentU, Vec3 tangentV, Vec3 normal) {
	// Untimatelly this requires solving the system of equations a0 tV + a1 tU = v so I don't think there is any better way of doing this.
	const auto v0 = tangentU.normalized();
	const auto v1 = cross(tangentU, normal).normalized();
	auto toOrthonormalBasis = [&](Vec3 v) -> Vec2 {
		return Vec2(dot(v, v0), dot(v, v1));
	};
	// Instead of doing this could for example use the Moore Penrose pseudoinverse or some other method for system with no solution. One advantage of this might be that it always gives some value instead of doing division by zero in points where the surface is not regular, but it is probably also more expensive, because it requires an inverse of a 3x3 matrix.
	const auto tU = toOrthonormalBasis(tangentU);
	const auto tV = toOrthonormalBasis(tangentV);
	const auto i = toOrthonormalBasis(v);
	const auto inUvCoordinates = Mat2(tU, tV).inversed() * i;
	return inUvCoordinates;
}

Vec2 vectorInTangentSpaceBasis(Vec3 v, Vec3 tangentU, Vec3 tangentV) {
	return vectorInTangentSpaceBasis(v, tangentU, tangentV, cross(tangentU, tangentV).normalized());
}

SurfaceTangent SurfaceTangent::makeUv(Vec2 uv) {
	return SurfaceTangent(uv);
}

SurfaceTangent::SurfaceTangent(Vec2 uv)
	: uv(uv) {}
