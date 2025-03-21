#pragma once

#include <vector>
#include <engine/Math/Vec3.hpp>

template<typename T>
void getTriangle(const std::vector<T>& values, const std::vector<i32>& indices, T* triangle, i32 triangleIndex) {
	for (i32 i = 0; i < 3; i++) {
		const auto index = indices[triangleIndex * 3 + i];
		triangle[i] = values[index];
	}
};

void indicesAddTri(std::vector<i32>& indicies, i32 i0, i32 i1, i32 i2);
void indicesAddQuad(std::vector<i32>& indicies, i32 i00, i32 i01, i32 i11, i32 i10);