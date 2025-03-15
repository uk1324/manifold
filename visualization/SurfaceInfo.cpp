#include "SurfaceInfo.hpp"

void SurfaceData::sortTriangles(Vec3 cameraPosition) {
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

i32 SurfaceData::vertexCount() const {
	return i32(positions.size());
}

i32 SurfaceData::triangleCount() const {
	return i32(indices.size() / 3);
}

void SurfaceData::addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt) {
	positions.push_back(p);
	normals.push_back(n);
	uvs.push_back(uv);
	uvts.push_back(uvt);
}