#include "World.hpp"
#include "Body.hpp"
#include "Joint.hpp"
#include <game/4d.hpp>
#include <imgui/imgui.h>

using std::vector;
using std::map;
using std::pair;

typedef map<ArbiterKey, Arbiter>::iterator ArbIter;
typedef pair<ArbiterKey, Arbiter> ArbPair;

bool World::accumulateImpulses = true;
bool World::warmStarting = true;
bool World::positionCorrection = true;

void World::clear() {
	bodies.clear();
	//joints.clear();
	arbiters.clear();
}

void World::broadPhase() {
	for (i32 i = 0; i < bodies.size(); i++) {
		Body* bi = bodies[i];

		for (i32 j = i + 1; j < bodies.size(); j++) {
			Body* bj = bodies[j];

			const auto bothStatic = bi->invMass == 0.0f && bj->invMass == 0.0f;
			if (bothStatic) {
				continue;
			}

			Arbiter newArb(bi, bj);
			ArbiterKey key(bi, bj);

			if (newArb.numContacts > 0) {
				ArbIter iter = arbiters.find(key);
				if (iter == arbiters.end()) {
					arbiters.insert(ArbPair(key, newArb));
				} else {
					iter->second.Update(newArb.contacts, newArb.numContacts);
				}
			} else {
				arbiters.erase(key);
			}
		}
	}
}

void World::settingsGui() {
	ImGui::SliderFloat("resistance", &resistance, 0.0f, 1.0f);
}

void World::step(f32 dt) {
	const f32 invDt = dt > 0.0f ? 1.0f / dt : 0.0f;

	// Determine overlapping bodies and update contact points.
	broadPhase();

	// Integrate forces.
	for (i32 i = 0; i < bodies.size(); i++) {
		Body* b = bodies[i];
		if (b->invMass == 0.0f) {
			b->velocity = Vec4(0.0f);
			b->position = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
			continue;
		}

		//Vec4 gravity(0.0f, 0.0f, 0.0f, -1.0f);
		Vec4 gravity(0.0f, 0.0f, 0.0f, 0.0f);

		// It doesn't matter if the parts of vectors that are outside the tangent space are removed before or after adding, because the removing is linear.
		Vec4 a = dt * (gravity + b->invMass * b->force);
		//a = projectVectorToSphereTangentSpace(b->position, a);
		b->velocity += a;
		b->velocity *= resistance;
		//b->velocity *= 0.99f;
		b->velocity = projectVectorToSphereTangentSpace(b->position, b->velocity);
		//b->velocity += dt * (gravity + b->invMass * b->force);
		//b->angularVelocity += dt * b->invI * b->torque;
	}

	// Perform pre-steps.
	for (auto& [key, arbiter] : arbiters) {
		arbiter.PreStep(invDt);
	}
	/*for (ArbIter arb = arbiters.begin(); arb != arbiters.end(); ++arb) {
	}*/

	/*for (int i = 0; i < (int)joints.size(); ++i)
	{
		joints[i]->PreStep(inv_dt);
	}*/

	// Perform iterations
	for (i32 i = 0; i < iterations; i++) {
		for (auto& [key, arbiter] : arbiters) {
			arbiter.applyImpulse();
		}
		/*for (ArbIter arb = arbiters.begin(); arb != arbiters.end(); ++arb) {
			arb->second.ApplyImpulse();
		}*/

		//for (int j = 0; j < (int)joints.size(); ++j)
		//{
		//	joints[j]->ApplyImpulse();
		//}
	}

	// Integrate Velocities
	for (auto& b : bodies) {
		const auto t = dot(b->position.normalized(), b->velocity.normalized());
		CHECK(t < 0.01f);

		//b->position += dt * b->velocity;
		//const auto 
		const auto movement = movementForwardOnSphere(b->position, b->velocity * dt);
		//if ((movement * b->position - b->position).length() > 0.1f) {
		//	int x = 5;
		//}

		b->position = moveForwardOnSphere(b->position, b->velocity * dt);
		/*b->position = movement * b->position;
		b->position = b->position.normalized();*/
		//b->position = quatMul(movement, b->position);
		//b->position = quatMul(b->position, movement);
		
		b->velocity = movement * b->velocity;
		//b->velocity = projectVectorToSphereTangentSpace(b->position, b->velocity);

		/*b->velocity = b->velocity*/

		/*const auto length = b->velocity.length();
		b->velocity = b->velocity.normalized();
		b->velocity *= length;*/
		//b->velocity = quatMul(movement, b->velocity);
		////b->velocity = quatMul(b->velocity, movement);
		// TODO: move velocity.
		//b->rotation += dt * b->angularVelocity;

		b->force = Vec4(0.0f);
		//b->torque = 0.0f;
	}
	for (int i = 0; i < (int)bodies.size(); ++i) {
		Body* b = bodies[i];
	}
}