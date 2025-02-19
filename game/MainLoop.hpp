#pragma once

#include <game/Renderer.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/SurfaceCamera.hpp>
#include <Array2d.hpp>
#include <game/PerlinNoise.hpp>
#include <game/Surfaces.hpp>
#include <game/GeodesicTool.hpp>
#include <game/CurvatureTool.hpp>

struct FlowParticles {
	std::vector<Vec2> positionsData;
	std::vector<Vec3> normalsData;
	std::vector<Vec3> colorsData;
	std::vector<Vec2> velocitiesData;
	Vec2& position(i32 particleIndex, i32 frame);
	Vec3& normal(i32 particleIndex, i32 frame);
	Vec3& color(i32 particleIndex, i32 frame);
	Vec2& velocity(i32 particleIndex, i32 frame);
	std::vector<i32> lifetime;
	std::vector<i32> elapsed;
	void initialize(i32 particleCount);
	i32 particleCount() const;
	void initializeParticle(i32 i, Vec2 position, Vec3 normal, i32 lifetime, Vec3 color, Vec2 velocity);
	static constexpr auto maxLifetime = 30;
};

template<typename F>
concept VectorField = requires(F vectorField, Vec3 v) {
	{ vectorField(v) } -> std::convertible_to<Vec3>;
};

struct MainLoop {
	MainLoop();
	void update();
	
	void gui();
	bool showGui = true;

	void initializeSelectedSurface();
	void initializeParticles(i32 particleCount);

	void calculateIntersections(Vec3 cameraPosition, Vec3 cameraForward);
	std::vector<MeshIntersection> intersections;

	GeodesicTool geodesicTool;

	FpsCamera3d fpsCamera;
	SufaceCamera surfaceCamera;
	struct SurfaceCameraUpdateResult {
		Mat4 view;
		Vec3 cameraPosition;
	};
	SurfaceCameraUpdateResult updateSurfaceCamera(f32 dt);

	CurvatureTool curvatureTool;

	std::vector<Vec2> uvPositions;

	struct Surface {
		std::vector<Vec3> positions;
		std::vector<Vec3> normals;
		std::vector<Vec2> uvs;
		// These always go from 0 to 1, could compute then on the fly instead of precomputing.
		std::vector<Vec2> uvts;
		std::vector<i32> indices;
		std::vector<f32> curvatures;
		f32 minCurvature;
		f32 maxCurvature;

		std::vector<Vec3> triangleCenters;
		std::vector<f32> triangleAreas;
		f32 totalArea;

		std::vector<i32> sortedTriangles;
		// Instead of using triangles could make a triangle fan or triangle strip.
		void sortTriangles(Vec3 cameraPosition);

		i32 vertexCount() const;
		i32 triangleCount() const;
		void addVertex(Vec3 p, Vec3 n, Vec2 uv, Vec2 uvt);
	};

	f32 transitionT = 1.0f;

	f32 meshOpacity = 0.5f;
	enum class MeshRenderMode {
		GRID,
		CURVATURE
	} meshRenderMode = MeshRenderMode::GRID;

	void initializeParticles(
		FlowParticles& particles,
		i32 particleCount,
		const RectParametrization auto& surface,
		const VectorField auto& vectorField);
	void updateParticles(const RectParametrization auto& surface, const VectorField auto& vectorField);

	std::random_device dev;
	std::default_random_engine rng;
	std::uniform_real_distribution<f32> uniform01;
	Vec2 randomPointOnSurface();

	FlowParticles flowParticles;
	void randomInitializeParticle(const RectParametrization auto& surface, i32 i);

	PerlinNoise noise;
	void randomizeVectorField(usize seed);

	Vec3 vectorFieldSample(Vec3 v) const;

	f32 vectorFieldMinLength = 0.0f;
	f32 vectorFieldMaxLength = 1.0f;
	//Vec3 vectorFieldSample(Vec3 pos, Vec2 uvPos, Vec3 tangentU, Vec3 tangentV, Vec3 normal) const;

	Surface surfaceData;

	

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::IN_SPACE;

	Renderer renderer;

	//Torus surface{ .r = 0.4f, .R = 2.0f };
	Surfaces surfaces;

	enum class VectorFieldType {
		RANDOM
	};
	VectorFieldType selectedVectorField = VectorFieldType::RANDOM;

	enum class ToolType {
		NONE,
		GEODESICS,
		CURVATURE,
		FLOW,
	} selectedTool = ToolType::NONE;
};