#include <game/Surface.hpp>
#include <game/Tri3d.hpp>
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

Vec3 Surface::position(Vec2 uv) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.position(uv.x, uv.y);
	}
}

Vec3 Surface::normal(Vec2 uv) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.normal(uv.x, uv.y);
	}
}

Vec3 Surface::tangentU(Vec2 uv) const{
	switch (selected) {
		using enum Type;
	case TORUS: return torus.tangentU(uv.x, uv.y);
	}
}

Vec3 Surface::tangentV(Vec2 uv) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.tangentV(uv.x, uv.y);
	}
}

ChristoffelSymbols Surface::christoffelSymbols(Vec2 uv) const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.christoffelSymbols(uv.x, uv.y);
	}
}

f32 Surface::uMin() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.uMin;
	}
}

f32 Surface::uMax() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.uMax;
	}
}

f32 Surface::vMin() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.vMin;
	}
}

f32 Surface::vMax() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.vMax;
	}
}

SquareSideConnectivity Surface::uConnectivity() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.uConnectivity;
	}
}

SquareSideConnectivity Surface::vConnectivity() const {
	switch (selected) {
		using enum Type;
	case TORUS: return torus.vConnectivity;
	}
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