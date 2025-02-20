#pragma once

#include <game/Renderer.hpp>
#include <game/SurfaceInfo.hpp>
#include <game/FpsCamera3d.hpp>
#include <game/SurfaceCamera.hpp>
#include <Array2d.hpp>
#include <game/Surfaces.hpp>
#include <game/SurfaceInfo.hpp>
#include <game/GeodesicTool.hpp>
#include <game/VectorFieldTool.hpp>
#include <game/CurvatureTool.hpp>
#include <game/Visualization4d.hpp>

struct MainLoop {
	MainLoop();
	void update();
	
	void gui();
	bool showGui = true;

	void initializeSelectedSurface();

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

	Visualization4d vis;

	VectorFieldTool vectorFieldTool;
	//std::vector<Vec2> uvPositions;

	f32 transitionT = 1.0f;

	f32 meshOpacity = 0.5f;
	enum class MeshRenderMode {
		GRID,
		CURVATURE
	} meshRenderMode = MeshRenderMode::GRID;

	//void initializeParticles(
	//	FlowParticles& particles,
	//	i32 particleCount,
	//	const RectParametrization auto& surface,
	//	const VectorField auto& vectorField);
	//void updateParticles(const RectParametrization auto& surface, const VectorField auto& vectorField);

	//Vec3 vectorFieldSample(Vec3 pos, Vec2 uvPos, Vec3 tangentU, Vec3 tangentV, Vec3 normal) const;

	SurfaceData surfaceData;

	enum class CameraMode {
		ON_SURFACE,
		IN_SPACE
	} cameraMode = CameraMode::IN_SPACE;

	Renderer renderer;

	Surfaces surfaces;

	enum class ToolType {
		NONE,
		GEODESICS,
		CURVATURE,
		FLOW,
	} selectedTool = ToolType::NONE;
};