#include "Arbiter.hpp"
#include "Body.hpp"
#include <game/4d.hpp>

// The normal points from A to B
//int Collide(Contact* contacts, Body* bodyA, Body* bodyB) {
//	const auto aPos = bodyA->position;
//	const auto bPos = bodyB->position;
//	const auto normal = bPos - aPos;
//	const auto distanceSquared = normal.lengthSquared();
//	if (distanceSquared > pow(bodyA->radius + bodyB->radius, 2.0f)) {
//		//return std::nullopt;
//		return 0;
//	}
//
//	//Collision collision;
//	//Contact collision;
//	auto& p = contacts[0];
//	i32 contactCount = 1;
//	//collision.contactCount = 1;
//	const auto distance = sqrt(distanceSquared);
//	p.normal = normal / distance;
//	p.separation = -(bodyA->radius + bodyB->radius - distance);
//	p.normal = (distanceSquared == 0.0f) ? Vec4(0.0f, 0.0f, 0.0f, 1.0f) : p.normal.normalized();
//	//p.pos = aPos + collision.normal * a.radius;
//	p.position = aPos + p.normal * bodyA->radius;
//	/*p.id = ContactPointId{ .featureOnA = ContactPointFeature::FACE, .featureOnAIndex = 0, .featureOnB = ContactPointFeature::FACE, .featureOnBIndex = 0 };*/
//	p.feature = FeaturePair{ .value = 0 };
//	return contactCount;
//}

#include <algorithm>
/*
The triangle is defined as the points p on the sphere that lie on the plane though the origin with normal = planeNormal.
And statisfying dot(p, edgeNormal0) >= 0, dot(p, edgeNormal1) >= 0, dot(p, edgeNormal2) >= 0

The intersection of a plane with the 3 sphere is a 2 sphere.

The closest point on this sphere to a point is the point projected onto the plane and normalized.
TODO: Proof
If you consider a regular sphere then this procedure would work. Simiarly for a circle.

If the projected point lies inside the triangle then it's the closest point.
Otherwise we can find the closest point on one of the segments.

The point is on a 2 sphere so we can ignore the 3 sphere and just focus on that.
The edge normals are orthogonal to the plane normal so they can be projected without going out of the 2 sphere.

Then after the projection if the point lies on an edge then that is the closest point. Otherwise it's one of the vertices. Just calculate the spherical distance to both and check which one is the closest.
*/
Vec4 closestPointOnTriangle(Vec4 planeNormal, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 point, Vec4 v0, Vec4 v1, Vec4 v2) {
	Vec4 pProjectedOntoSphere = point;
	pProjectedOntoSphere -= dot(planeNormal, pProjectedOntoSphere) * planeNormal;
	pProjectedOntoSphere = pProjectedOntoSphere.normalized();

	const auto d0 = dot(pProjectedOntoSphere, edgeNormal0);
	const auto d1 = dot(pProjectedOntoSphere, edgeNormal1);
	const auto d2 = dot(pProjectedOntoSphere, edgeNormal2);
	edgeNormal0 = edgeNormal0.normalized();
	edgeNormal1 = edgeNormal1.normalized();
	edgeNormal2 = edgeNormal2.normalized();
	if (d0 > 0.0f && d1 > 0.0f && d2 > 0.0f) {
		return pProjectedOntoSphere;
	}
	CHECK(abs(pProjectedOntoSphere.length() - 1.0f) < 0.01f);

	// TODO: Handle case when projected onto zero vector.
	auto closestPointToEdge = [&planeNormal](Vec4 v0, Vec4 v1, f32 d, Vec4 point, Vec4 edgeNormal) {
		Vec4 pProjectedOntoCircle = point;
		pProjectedOntoCircle -= dot(edgeNormal, point) * edgeNormal;
		pProjectedOntoCircle = pProjectedOntoCircle.normalized();
		CHECK(dot(pProjectedOntoCircle, planeNormal) < 0.01f);
		//return pProjectedOntoCircle;
		Vec4 edgeNormal0 = normalizedDirectionFromAToB(v0, v1);
		Vec4 edgeNormal1 = normalizedDirectionFromAToB(v1, v0);
		const auto d0 = dot(edgeNormal0, pProjectedOntoCircle);
		const auto d1 = dot(edgeNormal1, pProjectedOntoCircle);
		if (d0 >= 0.0f && d1 >= 0.0f) {
			return pProjectedOntoCircle;
		}
		const auto a0 = sphereAngularDistance(v0, pProjectedOntoCircle);
		const auto a1 = sphereAngularDistance(v1, pProjectedOntoCircle);
		if (a0 < a1) {
			return v0;
		} else {
			return v1;
		}
	};
	const auto t0 = dot(planeNormal, edgeNormal0);
	const auto t1 = dot(planeNormal, edgeNormal1);
	const auto t2 = dot(planeNormal, edgeNormal2);
	CHECK(t0 < 0.01f);
	CHECK(t1 < 0.01f);
	CHECK(t2 < 0.01f);

	//return pProjectedOntoSphere;

	Vec4 v[]{
		closestPointToEdge(v2, v0, d0, pProjectedOntoSphere, edgeNormal0),
		closestPointToEdge(v0, v1, d1, pProjectedOntoSphere, edgeNormal1),
		closestPointToEdge(v1, v2, d2, pProjectedOntoSphere, edgeNormal2),
	};
	std::ranges::sort(v, [&](Vec4 v0, Vec4 v1) {
		return sphereAngularDistance(point, v0) < sphereAngularDistance(point, v1);
	});
	return v[0];
}

//#include <algorithm>
//Vec4 closestPointOnTriangle(Vec4 planeNormal, Vec4 edgeNormal0, Vec4 edgeNormal1, Vec4 edgeNormal2, Vec4 point) {
//	Vec4 projectedP = point;
//	projectedP -= dot(planeNormal, projectedP) * planeNormal;
//	projectedP = projectedP.normalized();
//
//	const auto d0 = dot(projectedP, edgeNormal0);
//	const auto d1 = dot(projectedP, edgeNormal1);
//	const auto d2 = dot(projectedP, edgeNormal2);
//	if (d0 > 0.0f && d1 > 0.0f && d2 > 0.0f) {
//		return projectedP;
//	}
//
//	Vec4 v[]{
//		(projectedP - d0 * edgeNormal0).normalized(),
//		(projectedP - d2 * edgeNormal1).normalized(),
//		(projectedP - d1 * edgeNormal2).normalized(),
//	};
//	std::ranges::sort(v, [&](Vec4 v0, Vec4 v1) {
//		return (v0 - point).lengthSquared() < (v1 - point).lengthSquared();
//	});
//	return v[0];
//}

int collide2(Contact* contacts, Body* bodyA, Body* bodyB) {
	CHECK(bodyA->s);
	const auto p = closestPointOnTriangle(bodyA->planeNormal, bodyA->edgeNormal0, bodyA->edgeNormal1, bodyA->edgeNormal2, bodyB->position, bodyA->v0, bodyA->v1, bodyA->v2);

	auto& c = contacts[0];
	c.separation = sphereAngularDistance(p, bodyB->position);
	if (c.separation > bodyB->radius) {
		return 0;
	}
	c.position = p;

	c.normalAtPosition = (bodyB->position - p).normalized();
	//const auto t1 = c.normalAtPosition.length();
	//CHECK(abs(t1 - 1.0f) < 0.01f);
	c.normal = c.normalAtPosition;
	c.normalAToB = normalizedDirectionFromAToB(p, bodyB->position);
	//CHECK(abs(c.normalAToB.length() - 1.0f) < 0.01f);
	c.normalBToA = -normalizedDirectionFromAToB(bodyB->position, p);

	//c.normalAtPosition = normalizedDirectionFromAToB(p, bodyB->position);
	////const auto t1 = c.normalAtPosition.length();
	////CHECK(abs(t1 - 1.0f) < 0.01f);
	//c.normal = -c.normalAtPosition;
	//c.normalAToB = normalizedDirectionFromAToB(p, bodyB->position);
	////CHECK(abs(c.normalAToB.length() - 1.0f) < 0.01f);
	//c.normalBToA = -normalizedDirectionFromAToB(bodyB->position, p);
	//CHECK(abs(c.normalBToA.length() - 1.0f) < 0.01f);
	c.feature = FeaturePair{ .value = 0 };
	return 1;
}

// https://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf
int collide(Contact* contacts, Body* bodyA, Body* bodyB) {
	if (bodyA->s && bodyB->s) {
		return 0;
	}
	if (bodyA->s) {
		return collide2(contacts, bodyA, bodyB);
	} else {
		const auto r = collide2(contacts, bodyB, bodyA);
		auto& c = contacts[0];
		std::swap(c.normalAToB, c.normalBToA);
		c.normalAToB = -c.normalAToB;
		c.normalBToA = -c.normalBToA;
		c.normal = -c.normal;
		c.normalAtPosition = -c.normalAtPosition;
		return r;
	}

	const auto aPos = bodyA->position;
	const auto bPos = bodyB->position;
	const auto normal = bPos - aPos;
	//const auto distanceSquared = normal.lengthSquared();
	const auto distance = sphereAngularDistance(aPos, bPos);
	if (distance > bodyA->radius + bodyB->radius) {
		//return std::nullopt;
		return 0;
	}
	/*const auto dirFromAToB = normalizedDirectionFromAToB(aPos, bPos);
	const auto dirFromBToA = normalizedDirectionFromAToB(bPos, aPos);*/
	const auto dirFromAToB = normalizedDirectionFromAToB(aPos, bPos);
	const auto dirFromBToA = -normalizedDirectionFromAToB(bPos, aPos);
	{
		const auto t0 = dot(dirFromAToB, aPos);
		const auto t1 = dot(dirFromBToA, bPos);
		CHECK(t0 < 0.01f);
		CHECK(t1 < 0.01f);
	}
	//Collision collision;
	//Contact collision;
	auto& p = contacts[0];
	i32 contactCount = 1;
	//collision.contactCount = 1;
	//p.normal = normal / distance;
	p.normal = normal.normalized();
	p.normalAToB = dirFromAToB;
	p.normalBToA = dirFromBToA;
	p.separation = -(bodyA->radius + bodyB->radius - distance);
	p.normal = (distance == 0.0f) ? Vec4(0.0f, 0.0f, 0.0f, 1.0f) : p.normal.normalized();
	//p.pos = aPos + collision.normal * a.radius;
	//p.position = aPos + p.normal * bodyA->radius;
	//p.position = aPos + p.normal * bodyA->radius;
	//p.position = aPos + p.normal * bodyA->radius;
	p.position = moveForwardOnSphere(aPos, p.normal * bodyA->radius);
	p.normalAtPosition = normalizedDirectionFromAToB(p.position, bodyB->position);
	/*p.id = ContactPointId{ .featureOnA = ContactPointFeature::FACE, .featureOnAIndex = 0, .featureOnB = ContactPointFeature::FACE, .featureOnBIndex = 0 };*/
	p.feature = FeaturePair{ .value = 0 };
	return contactCount;
}

//int Collide(Contact* contacts, Body* bodyA, Body* bodyB) {
//	const auto aPos = bodyA->position;
//	const auto bPos = bodyB->position;
//	const auto normal = bPos - aPos;
//	//const auto distanceSquared = normal.lengthSquared();
//	const auto distanceSquared = dot(aPos, bPos);
//	if (distanceSquared > pow(bodyA->radius + bodyB->radius, 2.0f)) {
//		//return std::nullopt;
//		return 0;
//	}
//
//	//Collision collision;
//	//Contact collision;
//	auto& p = contacts[0];
//	i32 contactCount = 1;
//	//collision.contactCount = 1;
//	const auto distance = sqrt(distanceSquared);
//	p.normal = normal / distance;
//	p.separation = -(bodyA->radius + bodyB->radius - distance);
//	p.normal = (distanceSquared == 0.0f) ? Vec4(0.0f, 0.0f, 0.0f, 1.0f) : p.normal.normalized();
//	//p.pos = aPos + collision.normal * a.radius;
//	p.position = aPos + p.normal * bodyA->radius;
//	/*p.id = ContactPointId{ .featureOnA = ContactPointFeature::FACE, .featureOnAIndex = 0, .featureOnB = ContactPointFeature::FACE, .featureOnBIndex = 0 };*/
//	p.feature = FeaturePair{ .value = 0 };
//	return contactCount;
//}