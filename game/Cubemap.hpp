#pragma once

#include <game/Shaders/cubemapData.hpp>

namespace CubemapDirection {
	enum {
		POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEGATIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		COUNT = 6
	};
}

static const CubemapVertex cubeMapVertices[] = {
	Vec3(-1.0f, 1.0f, -1.0f),
	Vec3(-1.0f, -1.0f, -1.0f),
	Vec3(1.0f, -1.0f, -1.0f),
	Vec3(1.0f, -1.0f, -1.0f),
	Vec3(1.0f, 1.0f, -1.0f),
	Vec3(-1.0f, 1.0f, -1.0f),
	Vec3(-1.0f, -1.0f, 1.0f),
	Vec3(-1.0f, -1.0f, -1.0f),
	Vec3(-1.0f, 1.0f, -1.0f),
	Vec3(-1.0f, 1.0f, -1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(-1.0f, -1.0f, 1.0f),
	Vec3(1.0f, -1.0f, -1.0f),
	Vec3(1.0f, -1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f, -1.0f),
	Vec3(1.0f, -1.0f, -1.0f),
	Vec3(-1.0f, -1.0f, 1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f, -1.0f, 1.0f),
	Vec3(-1.0f, -1.0f, 1.0f),
	Vec3(-1.0f, 1.0f, -1.0f),
	Vec3(1.0f, 1.0f, -1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(1.0f, 1.0f, 1.0f),
	Vec3(-1.0f, 1.0f, 1.0f),
	Vec3(-1.0f, 1.0f, -1.0f),
	Vec3(-1.0f, -1.0f, -1.0f),
	Vec3(-1.0f, -1.0f, 1.0f),
	Vec3(1.0f, -1.0f, -1.0f),
	Vec3(1.0f, -1.0f, -1.0f),
	Vec3(-1.0f, -1.0f, 1.0f),
	Vec3(1.0f, -1.0f, 1.0f)
};

static i32 cubemapIndices[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35,
};