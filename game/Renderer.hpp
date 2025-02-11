#pragma once

#include <engine/Graphics/ShaderProgram.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <game/Camera3d.hpp>
#include <game/Shaders/basicShadingData.hpp>

struct Renderer {
	static Renderer make();

	void addTriangle(i32 i0, i32 i1, i32 i2);
	void addQuad(i32 i0, i32 i1, i32 i2, i32 i3);
	i32 addVertex(Vertex3Pn vertex);
	void triangle(Vertex3Pn v0, Vertex3Pn v1, Vertex3Pn v2);
	void renderTriangles();

	//Camera3d camera;
	Mat4 viewProjection;

	std::vector<i32> trianglesIndices;
	std::vector<Vertex3Pn> trianglesVertices;
	ShaderProgram& trianglesShader;
	Vbo trianglesVbo;
	Ibo trianglesIbo;
	Vao trianglesVao;

	Vbo instancesVbo;

};