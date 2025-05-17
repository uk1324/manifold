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

f32 angleBetween(Vec4 a, Vec4 b) {
	return acos(std::clamp(dot(a, b), -1.0f, 1.0f));
}

int Collide(Contact* contacts, Body* bodyA, Body* bodyB) {
	const auto aPos = bodyA->position;
	const auto bPos = bodyB->position;
	const auto normal = bPos - aPos;
	//const auto distanceSquared = normal.lengthSquared();
	const auto distance = angleBetween(aPos, bPos);
	if (distance > bodyA->radius + bodyB->radius) {
		//return std::nullopt;
		return 0;
	}
	const auto dirFromAToB = normalizedDirectionFromAToB(aPos, bPos);
	const auto dirFromBToA = normalizedDirectionFromAToB(aPos, bPos);
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