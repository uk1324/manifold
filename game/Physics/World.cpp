#include "World.hpp"
#include "Body.hpp"
#include <game/4d.hpp>
#include <imgui/imgui.h>

bool World::accumulateImpulses = true;
bool World::warmStarting = true;
bool World::positionCorrection = true;

void World::clear() {
	bodies.reset();
	contactConstraints.clear();
}

void World::broadPhase() {
	for (auto it = contactConstraints.begin(); it != contactConstraints.end();) {
		auto& key = it->first;
		if (!bodies.isAlive(key.body1) || !bodies.isAlive(key.body2)) {
			it = contactConstraints.erase(it);
		} else {
			++it;
		}
	}

	for (auto i = bodies.begin(); i != bodies.end(); ++i) {
		auto j = i;
		++j;
		for (; j != bodies.end(); ++j) {

			const auto bothStatic = i->invMass == 0.0f && j->invMass == 0.0f;
			if (bothStatic) {
				continue;
			}

			BodyIdPair key((*i).id, (*j).id);
			const auto b1 = bodies.get(key.body1);
			const auto b2 = bodies.get(key.body2);
			if (!b1.has_value() || !b2.has_value()) {
				CHECK_NOT_REACHED();
				continue;
			}
			ContactConstraint newArb(*b1, *b2);

			if (newArb.numContacts > 0) {
				auto iter = contactConstraints.find(key);
				if (iter == contactConstraints.end()) {
					contactConstraints.insert({ key, newArb });
				} else {
					iter->second.update(newArb.contacts, newArb.numContacts);
				}
			} else {
				contactConstraints.erase(key);
			}
		}
	}
}

void World::settingsGui() {
	ImGui::SliderFloat("resistance", &resistance, 0.0f, 1.0f);
	ImGui::InputFloat4("gravity", gravity.data());
}

void World::createSphere(Vec4 position, f32 radius, f32 mass) {
	auto body = bodies.create();
	body->set(radius, mass);
	body->position = position;
}

EntityArrayPair<Body> World::createWall(Vec4 v0, Vec4 v1, Vec4 v2, Vec4 edgeV2ToV0InwardNormal, Vec4 edgeV0ToV1InwardNormal, Vec4 edgeV1ToV2InwardNormal, Vec4 polygonPlaneNormal) {
	auto b = bodies.create();
	b->set(0.0f, INFINITY);
	b->s = true;

	b->edgeNormal0 = edgeV2ToV0InwardNormal;
	b->edgeNormal1 = edgeV0ToV1InwardNormal;
	b->edgeNormal2 = edgeV1ToV2InwardNormal;
	b->v0 = v0;
	b->v1 = v1;
	b->v2 = v2;
	b->planeNormal = polygonPlaneNormal;
	return b;
}

void World::step(f32 dt) {
	bodies.update();
	const f32 invDt = dt > 0.0f ? 1.0f / dt : 0.0f;

	broadPhase();

	for (auto b : bodies) {
		if (b->invMass == 0.0f) {
			b->velocity = Vec4(0.0f);
			b->position = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
			continue;
		}

		// It doesn't matter if the parts of vectors that are outside the tangent space are removed before or after adding, because the removing is linear.
		Vec4 a = dt * (gravity + b->invMass * b->force);
		b->velocity += a;
		b->velocity *= resistance;
		b->velocity = projectVectorToSphereTangentSpace(b->position, b->velocity);
	}

	for (auto& [key, constraint] : contactConstraints) {
		// @Performance: Maybe precompute the references to the bodies somewhere so this doesn't have to call get each time.
		auto a = bodies.get(key.body1);
		auto b = bodies.get(key.body2);
		if (!a.has_value() || !b.has_value()) {
			CHECK_NOT_REACHED();
			continue;
		}
			
		constraint.preStep(*a, *b, invDt);
	}

	for (i32 i = 0; i < iterations; i++) {
		for (auto& [key, constraint] : contactConstraints) {
			auto a = bodies.get(key.body1);
			auto b = bodies.get(key.body2);
			if (!a.has_value() || !b.has_value()) {
				CHECK_NOT_REACHED();
				continue;
			}

			constraint.applyImpulse(*a, *b);
		}
	}

	for (auto b : bodies) {
		b->velocity = projectVectorToSphereTangentSpace(b->position, b->velocity);
		const auto t = dot(b->position.normalized(), b->velocity.normalized());
		CHECK(t < 0.02f);
		// @Performance: This is probable doable using only quaternions instead of matrix multiplications.
		const auto movement = movementForwardOnSphere(b->position, b->velocity * dt);
		b->position = moveForwardOnSphere(b->position, b->velocity * dt);
		b->velocity = movement * b->velocity;
		b->force = Vec4(0.0f);
	}
}
