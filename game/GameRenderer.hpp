#pragma once

#include <game/TriangleRenderer.hpp>
#include <game/Shaders/surfaceData.hpp>

struct GameRenderer {
	static GameRenderer make();


	Mat4 transform;
	Mat4 view;

	TriangleRenderer<Vertex3Pnt> surfaceTriangles;
	ShaderProgram& surfaceShader;
	void renderSurfaceTriangles(f32 opacity);

	Vbo instancesVbo;
};