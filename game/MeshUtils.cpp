#include "MeshUtils.hpp"

void indicesAddTri(std::vector<i32>& indicies, i32 i0, i32 i1, i32 i2) {
	indicies.push_back(i0);
	indicies.push_back(i1);
	indicies.push_back(i2);
}

void indicesAddQuad(std::vector<i32>& indicies, i32 i00, i32 i01, i32 i11, i32 i10) {
	/*
	i01-i11
	|  /  |
	i00-i10
	*/
	indicies.push_back(i00);
	indicies.push_back(i10);
	indicies.push_back(i11);

	indicies.push_back(i00);
	indicies.push_back(i11);
	indicies.push_back(i01);
}