#pragma once

#include <engine/Math/Vec4.hpp>
#include <game/Physics/Body.hpp>

union FeaturePair {
	struct Edges
	{
		char inEdge1;
		char outEdge1;
		char inEdge2;
		char outEdge2;
	} e;
	int value;
};

struct Contact {
	Contact() : Pn(0.0f), Pt(0.0f), Pnb(0.0f) {}

	Vec4 normalAToB = Vec4(0.0f);
	Vec4 normalBToA = Vec4(0.0f);
	Vec4 position = Vec4(0.0f);
	Vec4 normal = Vec4(0.0f);
	Vec4 normalAtPosition = Vec4(0.0f);
	Vec4 r1 = Vec4(0.0f), r2 = Vec4(0.0f);
	float separation = 0.0f;
	float Pn = 0.0f;	// accumulated normal impulse
	float Pt = 0.0f;	// accumulated tangent impulse
	float Pnb = 0.0f;	// accumulated normal impulse for position bias
	float massNormal = 0.0f, massTangent = 0.0f;
	float bias = 0.0f;
	FeaturePair feature = FeaturePair{ .value = 0 };
};

struct ContactConstraint {
	enum { MAX_POINTS = 2 };

	ContactConstraint(const Body& b1, const Body b2);

	void update(Contact* contacts, i32 numContacts);

	void preStep(Body& body1, Body& body2, f32 inv_dt);
	void applyImpulse(Body& b1, Body& b2);

	Contact contacts[MAX_POINTS];
	i32 numContacts;
};

// Unordered pair. 2 element set.
struct BodyIdPair {
	BodyIdPair(BodyId b1, BodyId b2);

	BodyId body1;
	BodyId body2;
};

// Used by std::set.
bool operator<(const BodyIdPair& a1, const BodyIdPair& a2);

int collide(Contact* contacts, const Body& bodyA, const Body& bodyB);
Vec4 closestPointOnTriangle(Vec4 planeNormal, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 point, Vec4 v0, Vec4 v1, Vec4 v2);