#pragma once

#include <engine/Math/Vec4.hpp>

struct Body;

union FeaturePair
{
	struct Edges
	{
		char inEdge1;
		char outEdge1;
		char inEdge2;
		char outEdge2;
	} e;
	int value;
};

struct Contact
{
	Contact() : Pn(0.0f), Pt(0.0f), Pnb(0.0f) {}

	Vec4 normalAToB = Vec4(0.0f);
	Vec4 normalBToA = Vec4(0.0f);
	Vec4 position = Vec4(0.0f);
	Vec4 normal = Vec4(0.0f);
	Vec4 normalAtPosition = Vec4(0.0f);
	Vec4 r1 = Vec4(0.0f), r2 = Vec4(0.0f);
	float separation;
	float Pn;	// accumulated normal impulse
	float Pt;	// accumulated tangent impulse
	float Pnb;	// accumulated normal impulse for position bias
	float massNormal, massTangent;
	float bias;
	FeaturePair feature;
};

struct ArbiterKey
{
	ArbiterKey(Body* b1, Body* b2)
	{
		if (b1 < b2)
		{
			body1 = b1; body2 = b2;
		} else
		{
			body1 = b2; body2 = b1;
		}
	}

	Body* body1;
	Body* body2;
};

struct Arbiter
{
	enum { MAX_POINTS = 2 };

	Arbiter(Body* b1, Body* b2);

	void Update(Contact* contacts, int numContacts);

	void PreStep(float inv_dt);
	void applyImpulse();

	Contact contacts[MAX_POINTS];
	int numContacts;

	Body* body1;
	Body* body2;

	// Combined friction
	float friction;
};

// This is used by std::set
inline bool operator < (const ArbiterKey& a1, const ArbiterKey& a2)
{
	if (a1.body1 < a2.body1)
		return true;

	if (a1.body1 == a2.body1 && a1.body2 < a2.body2)
		return true;

	return false;
}

int collide(Contact* contacts, Body* body1, Body* body2);
Vec4 closestPointOnTriangle(Vec4 planeNormal, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 point, Vec4 v0, Vec4 v1, Vec4 v2);