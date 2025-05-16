#pragma once
#include <engine/Math/Vec4.hpp>

struct Body
{
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

	float friction;
	float mass, invMass;
	//float I, invI;
};
