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

// https://media.steampowered.com/apps/valve/2015/DirkGregorius_Contacts.pdf
int collide(Contact* contacts, Body* bodyA, Body* bodyB) {
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