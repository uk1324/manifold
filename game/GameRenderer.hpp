#pragma once

#include <game/TriangleRenderer.hpp>
#include <game/Shaders/surfaceData.hpp>
#include <game/Shaders/coloredData.hpp>
#include <game/Shaders/bulletData.hpp>
#include <game/Cubemap.hpp>

struct Mesh {
	Vbo vbo;
	Ibo ibo;
	Vao vao;
	i32 indexCount;
};

struct GameRenderer {
	static GameRenderer make();

	Mat4 transform;
	Mat4 view;
	Mat4 projection;

	Mesh cubemapMesh;
	ShaderProgram& cubemapShader;
	void renderCubemap();

	Mesh bulletRectMesh;
	static void drawRectMeshInstances(usize count);

	ShaderProgram& bulletShader;
	std::vector<BulletInstance> bulletInstances;
	void bullet(f32 size, Vec3 position, Vec4 color);
	void renderBullets(const Mat4& rotateMatrix);

	void initColoredShader();
	ShaderProgram& coloredShader;

	Mesh hemisphere;
	std::vector<ColoredInstance> hemispheres;
	void sphere(Vec3 center, f32 radius, Vec3 color);
	void renderHemispheres();

	TriangleRenderer<Vertex3Pnt> surfaceTriangles;
	ShaderProgram& surfaceShader;
	void renderSurfaceTriangles(f32 opacity);

	Vbo instancesVbo;
};