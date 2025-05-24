#pragma once
#include <vector>
#include <map>
#include <engine/Math/Vec4.hpp>
#include "ContactConstraint.hpp"
#include <game/EntityArray.hpp>
#include <game/Physics/Body.hpp>
//void initializeBodyIdPair(BodyId& a, BodyId& b);

struct World {
	World(i32 iterations): iterations(iterations) {}

	void clear();

	void step(f32 dt);

	void broadPhase();

	f32 resistance = 0.97f;
	Vec4 gravity = Vec4(0.0f);
	void settingsGui();

	//std::vector<Body*> bodies;
	BodyArray bodies;
	void createSphere(Vec4 position, f32 radius, f32 mass);
	//std::vector<Joint*> joints;
	std::map<BodyIdPair, ContactConstraint> arbiters;
	i32 iterations;
	static bool accumulateImpulses;
	static bool warmStarting;
	static bool positionCorrection;
};

