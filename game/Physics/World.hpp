#pragma once
#include <vector>
#include <map>
#include <engine/Math/Vec4.hpp>
#include "Arbiter.hpp"

struct Body;
struct Joint;

struct World
{
	World(int iterations): iterations(iterations) {}

	void Add(Body* body);
	void Add(Joint* joint);
	void Clear();

	void Step(float dt);

	void BroadPhase();

	std::vector<Body*> bodies;
	std::vector<Joint*> joints;
	std::map<ArbiterKey, Arbiter> arbiters;
	int iterations;
	static bool accumulateImpulses;
	static bool warmStarting;
	static bool positionCorrection;
};

