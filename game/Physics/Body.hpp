#pragma once
#include <engine/Math/Vec4.hpp>
#include <game/EntityArray.hpp>

struct Body {
	Body();
	void set(float radiusToSet, float massToSet);

	Vec4 position;
	//float rotation;

	Vec4 velocity;
	//float angularVelocity;

	Vec4 force;
	//float torque;

	f32 radius;
	//Vec2 width;

	bool s = false;
	Vec4 planeNormal = Vec4(0.0f);
	Vec4 v0 = Vec4(0.0f);
	Vec4 v1 = Vec4(0.0f);
	Vec4 v2 = Vec4(0.0f);
	Vec4 edgeNormal0 = Vec4(0.0f);
	Vec4 edgeNormal1 = Vec4(0.0f);
	Vec4 edgeNormal2 = Vec4(0.0f);

	float friction;
	float mass, invMass;
	//float I, invI;
};

struct BodyDefaultInitialize {
	Body operator()();
};

using BodyArray = EntityArray<Body, BodyDefaultInitialize>;
using BodyId = EntityArrayId<Body>;