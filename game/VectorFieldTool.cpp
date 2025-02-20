#include "VectorFieldTool.hpp"
#include <glad/glad.h>
#include <engine/Input/Input.hpp>
#include <game/RayIntersection.hpp>
#include <game/MeshUtils.hpp>
#include <game/Tri3d.hpp>
#include <game/Constants.hpp>
#include <game/Utils.hpp>
#include <engine/Math/Color.hpp>

VectorFieldTool::VectorFieldTool()
	: noise(PerlinNoise(5)) {
	sources.push_back(Source{ SourceType::SOURCE, Vec3(0.5f) });
}

void VectorFieldTool::initializeSampleVectors(const SurfaceData& surfaceData, const Surfaces& surfaces) {
	sampleVectors.clear();
	for (i32 i = 0; i < 5000; i++) {
		const auto posUv = randomPointOnSurface(surfaceData);
		const auto pos = surfaces.position(posUv);
		const auto position = surfaces.position(posUv);
		const auto tangentU = surfaces.tangentU(posUv);
		const auto tangentV = surfaces.tangentV(posUv);
		const auto normal = cross(tangentU, tangentV).normalized();
		const auto vectorUnprojected = randomVectorFieldSample(position);
		const auto vectorUv = vectorInTangentSpaceBasis(vectorUnprojected, tangentU, tangentV, normal);
		const auto vector = vectorUv.x * tangentU + vectorUv.y * tangentV;
		sampleVectors.push_back(SampleVector{
			.color = Color3::scientificColoring(vector.length(), vectorFieldMinLength, vectorFieldMaxLength),
			.position = pos,
			.direction = vector,
		});
	}
}

void VectorFieldTool::initializeParticles(
	FlowParticles& particles,
	i32 particleCount,
	const RectParametrization auto& surface,
	const SurfaceData& surfaceData,
	const VectorField auto& vectorField) {

	particles.initialize(particleCount);
	for (i32 i = 0; i < particleCount; i++) {
		randomInitializeParticle(surface, surfaceData, vectorField, i);
	}
	for (i32 i = 0; i < particleCount; i++) {
		const auto elapsed = std::uniform_int_distribution<i32>(0, particles.lifetime[i] - 1)(rng);
		for (i32 j = 1; j <= elapsed; j++) {
			particles.position(i, j) = particles.position(i, 0);
		}
		particles.elapsed[i] = elapsed;
	}

}

void VectorFieldTool::initializeParticles(const Surfaces& surfaces, const SurfaceData& surfaceData, i32 particleCount) {
	#define I(surface, vectorField) initializeParticles(flowParticles, particleCount, surface, surfaceData, vectorField)
	#define S(vectorField) \
	switch (surfaces.selected) { \
		using enum Surfaces::Type; \
	case TORUS: I(surfaces.torus, vectorField); break; \
	case TREFOIL: I(surfaces.trefoil, vectorField); break; \
	case HELICOID: I(surfaces.helicoid, vectorField); break; \
	case MOBIUS_STRIP: I(surfaces.mobiusStrip, vectorField); break; \
	case PSEUDOSPHERE: I(surfaces.pseudosphere, vectorField); break; \
	case CONE: I(surfaces.cone, vectorField); break; \
	case SPHERE: I(surfaces.sphere, vectorField); break; \
	case PROJECTIVE_PLANE: I(surfaces.projectivePlane, vectorField); break; \
	}
	switch (selectedVectorField) {
		using enum VectorFieldType;
	case RANDOM: S([this](Vec3 pos) { return randomVectorFieldSample(pos); }); break;
	case CUSTOM: S(CustomVectorField(this)); break;
	}
	#undef I
	#undef S

	initializeSampleVectors(surfaceData, surfaces);
}

void VectorFieldTool::initializeValues(const Surfaces& surfaces, const SurfaceData& surfaceData) {
	vectorFieldMinLength = std::numeric_limits<f32>::infinity();
	vectorFieldMaxLength = -std::numeric_limits<f32>::infinity();
	for (i32 i = 0; i < surfaceData.triangleCount(); i++) {
		Vec3 v[3];
		getTriangle(surfaceData.positions, surfaceData.indices, v, i);
		const auto normal = cross(v[1] - v[0], v[2] - v[0]).normalized();
		const auto centroid = (v[0] + v[1] + v[2]) / 3.0f;
		const auto vector = selectedVectorFieldSample(centroid);
		const auto projectedOntoTangentSpace = vector - dot(vector, normal) * normal;
		const auto length = projectedOntoTangentSpace.lengthSquared();
		if (length > vectorFieldMaxLength) {
			vectorFieldMaxLength = length;
		}
		if (length < vectorFieldMinLength) {
			vectorFieldMinLength = length;
		}
	}
	vectorFieldMaxLength = sqrt(vectorFieldMaxLength);
	vectorFieldMinLength = sqrt(vectorFieldMinLength);
	initializeSampleVectors(surfaceData, surfaces);
}

void VectorFieldTool::updateParticles(
	const Mat4& view,
	Renderer& renderer,
	const RectParametrization auto& surface, 
	const SurfaceData& surfaceData, 
	const VectorField auto& vectorField) {
	//	auto particle = [this](Vec3 v, f32 a, f32 size, Vec3 color) {
	//	renderer.flowParticle(size, v, Vec4(color, a));
	//};
	static bool stopped = false;
	//ImGui::Checkbox("stopped", &stopped);
	bool step = !stopped;
	/*if (ImGui::Button("step")) {
		step = true;
	}*/
	// Updating every other frame makes it look laggy.
	// creation frame = 0
	// normally updates for lifetime frames. On frame = lifetime - 1 is the last update.
	// When frame >= lifetime then the it starts disappearing. 
	// On frame lifetime + disappearTime it would fully disappear so instead it's respawned.
	const auto disappearTime = 10;
	for (i32 i = 0; i < flowParticles.particleCount(); i++) {
		const auto& lifetime = flowParticles.lifetime[i];
		auto& elapsed = flowParticles.elapsed[i];
		if (step) {
			elapsed++;
			const auto disapperElapsed = std::max(0, elapsed - lifetime);
			if (elapsed < lifetime) {
				const auto p = flowParticles.position(i, elapsed - 1);
				const auto velocity = flowParticles.velocity(i, elapsed - 1);
				const auto newPosition = p + velocity * Constants::dt * 3.0f;
				flowParticles.position(i, elapsed) = newPosition;
				flowParticles.normal(i, elapsed) = surface.normal(newPosition.x, newPosition.y);
				{
					const auto newPosition3 = surface.position(newPosition.x, newPosition.y);
					const auto tangentU = surface.tangentU(newPosition.x, newPosition.y);
					const auto tangentV = surface.tangentV(newPosition.x, newPosition.y);

					const auto normal = flowParticles.normal(i, elapsed - 1);
					const auto vector = vectorField(newPosition3);
					auto vectorUv = vectorInTangentSpaceBasis(vector, tangentU, tangentV, normal);
					const auto l = (vectorUv.x * tangentU + vectorUv.y * tangentV).length();
					const auto color = Color3::scientificColoring(l, vectorFieldMinLength, vectorFieldMaxLength);
					flowParticles.color(i, elapsed) = color;
					flowParticles.velocity(i, elapsed) = vectorUv;
				}
			} else if (disapperElapsed >= disappearTime - 1) {
				randomInitializeParticle(surface, surfaceData, vectorField, i);
			}
		}
		const auto disapperElapsed = std::max(0, elapsed - lifetime);


		const auto frameCount = elapsed + 1;
		for (i32 positionI = 0; positionI < std::min(frameCount, lifetime); positionI++) {
			const auto disappearT = f32(disapperElapsed) / f32(disappearTime);
			const auto p = flowParticles.position(i, positionI);
			f32 a = 0.5f;
			f32 t = 1.0f;
			t *= f32(positionI + 1) / f32(frameCount);
			t *= 1.0f - disappearT;
			const auto maxSize = 0.03f;
			const auto size = (t + 1.0f) / 2.0f * maxSize;
			auto position = surface.position(p.x, p.y);
			const auto normal = flowParticles.normal(i, positionI);
			const auto color = flowParticles.color(i, positionI);
			position += flowParticles.normal(i, positionI) * maxSize;

			renderer.flowParticle(size, position, Vec4(color, a * t));
		}

	}

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	renderer.renderFlowParticles(view);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

Vec2 VectorFieldTool::randomPointOnSurface(const SurfaceData& surfaceData) {
	const auto value = uniform01(rng) * surfaceData.totalArea;
	f32 cursor = 0.0f;
	i32 randomTriangleIndex = 0;
	for (i32 i = 0; i < surfaceData.triangleCount(); i++) {
		cursor += surfaceData.triangleAreas[i];
		if (cursor >= value) {
			randomTriangleIndex = i;
			break;
		}
	}
	Vec2 uvs[3];
	getTriangle(surfaceData.uvs, surfaceData.indices, uvs, randomTriangleIndex);
	const auto r0 = uniform01(rng);
	const auto r1 = uniform01(rng);
	return uniformRandomPointOnTri(uvs, r0, r1);
}

void VectorFieldTool::randomInitializeParticle(const RectParametrization auto& surface, const SurfaceData& surfaceData, const VectorField auto& vectorField, i32 i) {
	const auto p = randomPointOnSurface(surfaceData);
	const auto lifetime = std::uniform_int_distribution<i32>(
		i32(FlowParticles::maxLifetime * f32(0.7f)),
		FlowParticles::maxLifetime)(rng);

	const auto position = surface.position(p.x, p.y);
	const auto tangentU = surface.tangentU(p.x, p.y);
	const auto tangentV = surface.tangentV(p.x, p.y);
	const auto normal = cross(tangentU, tangentV).normalized();
	const auto vector = vectorField(position);
	auto vectorUv = vectorInTangentSpaceBasis(vector, tangentU, tangentV, normal);
	const auto l = (vectorUv.x * tangentU + vectorUv.y * tangentV).length();
	const auto color = Color3::scientificColoring(l, vectorFieldMinLength, vectorFieldMaxLength);

	//const auto lifetime = FlowParticles::maxLifetime;
	flowParticles.initializeParticle(i, p, normal, lifetime, color, vectorUv);
}

void VectorFieldTool::randomizeVectorField(usize seed, const SurfaceData& surfaceData, const Surfaces& surfaces) {
	noise = PerlinNoise(seed);
	initializeValues(surfaces, surfaceData);
}

void VectorFieldTool::randomizeVectorField(const SurfaceData& surfaceData, const Surfaces& surfaces) {
	randomizeVectorField(rng(), surfaceData, surfaces);
}

Vec3 VectorFieldTool::randomVectorFieldSample(Vec3 v) const {
	v /= 10.0f;
	return Vec3(
		noise.value3d(v),
		noise.value3d(v + Vec3(214.0f, 0.0f, 0.0f)),
		noise.value3d(v + Vec3(0.0f, 24.456f, 0.0f))
	);
}

Vec3 VectorFieldTool::selectedVectorFieldSample(Vec3 v) const {
	switch (selectedVectorField) {
		using enum VectorFieldType;
	case RANDOM: return randomVectorFieldSample(v);
	case CUSTOM: return customVectorFieldSample(v);
	}
	return Vec3(0.0f);
}

Vec3 VectorFieldTool::customVectorFieldSample(Vec3 v) const {
	Vec3 value(0.0f);
	for (const auto& source : sources) {
		value += (v - source.pos) / 10.0f;
	}
	return value;
}

void VectorFieldTool::update(const Mat4& view, Vec3 cameraPosition, Vec3 cameraDirection, Renderer& renderer, const Surfaces& surfaces, const SurfaceData& surfaceData) {
	if (showFlow) {
		 #define I(name, vectorField) updateParticles(view, renderer, surfaces.name, surfaceData, vectorField)
		#define S(vectorField) \
		switch (surfaces.selected) { \
			using enum Surfaces::Type; \
		case TORUS: I(torus, vectorField); break; \
		case TREFOIL: I(trefoil, vectorField); break; \
		case HELICOID: I(helicoid, vectorField); break; \
		case MOBIUS_STRIP: I(mobiusStrip, vectorField); break; \
		case PSEUDOSPHERE: I(pseudosphere, vectorField); break; \
		case CONE: I(cone, vectorField); break; \
		case SPHERE: I(sphere, vectorField); break; \
		case PROJECTIVE_PLANE: I(projectivePlane, vectorField); break; \
		}
		switch (selectedVectorField) {
			using enum VectorFieldType;
		case RANDOM: S([this](Vec3 pos) { return randomVectorFieldSample(pos); }); break;
		case CUSTOM: S(CustomVectorField(this)); break;
		}
		#undef I
		#undef S
	}

	if (showVectors) {
		for (const auto& vector : sampleVectors) {
			const auto radius = 0.01f;
			const auto coneRadius = 2.0f * radius;
			const auto coneLength = 2.0f * coneRadius;
			renderer.arrowStartEnd(vector.position, vector.position + vector.direction, radius, coneRadius, coneLength, vector.color, vector.color);
		}
	}
	if (selectedVectorField == VectorFieldType::CUSTOM) {
		vectorFieldEditorUpdate(renderer, cameraPosition, cameraDirection, surfaces, surfaceData);
	}
}

void VectorFieldTool::vectorFieldEditorUpdate(Renderer& renderer, Vec3 cameraPosition, Vec3 cameraDirection, const Surfaces& surfaces, const SurfaceData& surfaceData) {
	f32 radius = 0.05f;

	struct Hit {
		f32 t;
		i32 index;
	};
	const Ray ray(cameraPosition, cameraDirection);
	std::optional<Hit> closestHit;
	for (i32 i = 0; i < sources.size(); i++) {
		const auto& source = sources[i];
		const auto intersection = raySphereIntersection(cameraPosition, cameraDirection, source.pos, radius);
		if (!intersection.has_value()) {
			continue;
		}

		if (!closestHit.has_value() || *intersection < closestHit->t) {
			closestHit = Hit{ 
				.t = *intersection,
				.index = i, 
			};
		}
	}

	if (Input::isMouseButtonDown(MouseButton::LEFT) && closestHit.has_value()) {
		const auto& source = sources[closestHit->index];
		const auto distance = cameraPosition.distanceTo(source.pos);
		grabbedSource = GrabbedSource{
			.index = closestHit->index,
			.distance = distance,
		};
	}

	if (Input::isMouseButtonHeld(MouseButton::LEFT) && grabbedSource.has_value()) {
		auto& source = sources[grabbedSource->index];
		source.pos = ray.at(grabbedSource->distance);
		initializeValues(surfaces, surfaceData);
	}

	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		grabbedSource = std::nullopt;
	}

	for (const auto& source : sources) {
		renderer.sphere(source.pos, 0.05f, Color3::WHITE);
	}
}

Vec2& FlowParticles::position(i32 particleIndex, i32 frame) {
	ASSERT(particleIndex < particleCount());
	if (lifetime[particleIndex] > 0) {
		ASSERT(frame < lifetime[particleIndex]);
	}
	return positionsData[maxLifetime * particleIndex + frame];
}

Vec3& FlowParticles::normal(i32 particleIndex, i32 frame) {
	return normalsData[maxLifetime * particleIndex + frame];
}

Vec3& FlowParticles::color(i32 particleIndex, i32 frame) {
	return colorsData[maxLifetime * particleIndex + frame];
}

Vec2& FlowParticles::velocity(i32 particleIndex, i32 frame) {
	return velocitiesData[maxLifetime * particleIndex + frame];
}

void FlowParticles::initialize(i32 particleCount) {
	positionsData.resize(particleCount * maxLifetime);
	normalsData.resize(particleCount * maxLifetime);
	velocitiesData.resize(particleCount * maxLifetime);
	colorsData.resize(particleCount * maxLifetime);
	lifetime.resize(particleCount);
	elapsed.resize(particleCount);
}

i32 FlowParticles::particleCount() const {
	return i32(lifetime.size());
}

void FlowParticles::initializeParticle(i32 i, Vec2 position, Vec3 normal, i32 lifetime, Vec3 color, Vec2 velocity) {
	this->position(i, 0) = position;
	this->normal(i, 0) = normal;
	this->velocity(i, 0) = velocity;
	this->color(i, 0) = color;
	this->lifetime[i] = lifetime;
	this->elapsed[i] = 0;
}

VectorFieldTool::CustomVectorField::CustomVectorField(const VectorFieldTool* self)
	: self(*self) {}

Vec3 VectorFieldTool::CustomVectorField::operator()(Vec3 v) const {
	return self.customVectorFieldSample(v);
}
