#pragma once

#include <game/Surfaces/RectParametrization.hpp>
#include <vector>
#include <random>
#include <game/Surfaces.hpp>
#include <game/SurfaceInfo.hpp>
#include <game/Renderer.hpp>
#include <game/PerlinNoise.hpp>

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

struct VectorFieldTool {
	VectorFieldTool();

	FlowParticles flowParticles;
	struct SampleVector {
		Vec3 color;
		Vec3 position;
		Vec3 direction;
	};
	std::vector<SampleVector> sampleVectors;
	void initializeSampleVectors(const SurfaceData& data, const Surfaces& surfaces);

	bool showFlow = true;
	bool showVectors = false;

	void initializeParticles(
		FlowParticles& particles,
		i32 particleCount,
		const RectParametrization auto& surface,
		const SurfaceData& surfaceData,
		const VectorField auto& vectorField);
	void initializeParticles(const Surfaces& surfaces, const SurfaceData& surfaceData, i32 particleCount);

	void initializeValues(const Surfaces& surfaces, const SurfaceData& surfaceData);

	void updateParticles(
		const Mat4& view,
		Renderer& renderer,
		const RectParametrization auto& surface, 
		const SurfaceData& surfaceData,
		const VectorField auto& vectorField);

	enum class VectorFieldType {
		RANDOM,
		CUSTOM,
	};
	VectorFieldType selectedVectorField = VectorFieldType::RANDOM;

	std::random_device dev;
	std::default_random_engine rng;
	std::uniform_real_distribution<f32> uniform01;
	Vec2 randomPointOnSurface(const SurfaceData& surfaceData);

	//FlowParticles flowParticles;
	void randomInitializeParticle(const RectParametrization auto& surface, const SurfaceData& surfaceData, const VectorField auto& vectorField, i32 i);

	PerlinNoise noise;
	void randomizeVectorField(usize seed, const SurfaceData& surfaceData, const Surfaces& surfaces);
	void randomizeVectorField(const SurfaceData& surfaceData, const Surfaces& surfaces);

	Vec3 randomVectorFieldSample(Vec3 v) const;

	Vec3 selectedVectorFieldSample(Vec3 v) const;

	Vec3 customVectorFieldSample(Vec3 v) const;
	struct CustomVectorField {
		CustomVectorField(const VectorFieldTool* self);
		const VectorFieldTool& self;
		Vec3 operator()(Vec3 v) const;
	};
	enum class SourceType {
		SOURCE, SINK
	};
	struct Source {
		SourceType type;
		Vec3 pos;
	};
	std::vector<Source> sources;
	struct GrabbedSource {
		i32 index;
		f32 distance;
	};
	std::optional<GrabbedSource> grabbedSource;

	void update(const Mat4& view, Vec3 cameraPosition, Vec3 cameraDirection, Renderer& renderer, const Surfaces& surfaces, const SurfaceData& surfaceData);
	void vectorFieldEditorUpdate(Renderer& renderer, Vec3 cameraPosition, Vec3 cameraDirection, const Surfaces& surfaces, const SurfaceData& surfaceData);

	f32 vectorFieldMinLength = 0.0f;
	f32 vectorFieldMaxLength = 1.0f;
};