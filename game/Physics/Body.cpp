#include "Body.hpp"

Body::Body()
	: position(Vec4(0.0f))
	, velocity(Vec4(0.0f))
	, force(Vec4(0.0f)) {
	//position.Set(0.0f, 0.0f);
	//rotation = 0.0f;
	//velocity.Set(0.0f, 0.0f);
	//angularVelocity = 0.0f;
	//force.Set(0.0f, 0.0f);
	//torque = 0.0f;
	friction = 0.2f;

	//width.Set(1.0f, 1.0f);
	mass = FLT_MAX;
	invMass = 0.0f;
	//I = FLT_MAX;
	//invI = 0.0f;
}

void Body::set(float radiusToSet, float massToSet) {
	/*position.Set(0.0f, 0.0f);
	rotation = 0.0f;
	velocity.Set(0.0f, 0.0f);
	angularVelocity = 0.0f;
	force.Set(0.0f, 0.0f);
	torque = 0.0f;
	friction = 0.2f;*/

	radius = radiusToSet;
	mass = massToSet;
	//width = w;
	//mass = m;

	if (mass < FLT_MAX)
	{
		invMass = 1.0f / mass;
		//I = mass * (width.x * width.x + width.y * width.y) / 12.0f;
		//invI = 1.0f / I;
	} else
	{
		invMass = 0.0f;
		//I = FLT_MAX;
		//invI = 0.0f;
	}
}