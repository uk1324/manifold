#pragma once
#include <vector>
#include <map>
#include <engine/Math/Vec4.hpp>
#include "Arbiter.hpp"

struct Body;
//struct Joint;

struct World
{
	World(i32 iterations): iterations(iterations) {}

	void clear();

	void step(f32 dt);

	void broadPhase();

	f32 resistance = 0.97f;
	void settingsGui();

	std::vector<Body*> bodies;
	//std::vector<Joint*> joints;
	std::map<ArbiterKey, Arbiter> arbiters;
	i32 iterations;
	static bool accumulateImpulses;
	static bool warmStarting;
	static bool positionCorrection;
};

