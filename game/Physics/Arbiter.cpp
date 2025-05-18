#include "Arbiter.hpp"
#include "Body.hpp"
#include "World.hpp"
#include <algorithm>
#include <engine/Math/Vec2.hpp>
#include <game/Math.hpp>
#include <game/4d.hpp>

/*
Calculating relative velocity.
When the center of a sphere moves along a geodesic not all points of the body move along geodesics.
We can imagine a 2d case. That is we have a sphere and a spherical circle moving on it. The movement coresponds to a rotation around an axis. So the velocity of points depends on their distance from the fixed axis.
It seems that in 4D resonable that the velocity of points will depend on the distance to the fixed plane.

Let's consider a orthogonal matrix in the canonical basis.
[R 0 0]
[0 1 0]
[0 0 1]
We can ignore the fixed axes and only consider then rotating subspace. Then in this space the velocity depends on the distance to the origin.

First projection to the 2D subpace and the calculating the distance from the origin seems equivalent to just calculating the distance to the fixed plane orthogonal to the 2D subspace, because a projection orthogonal to the subspace doesn't change the distance in that subspace i think?


The velocity vector has to lie in the tangent space that is it has to be orthogonal to the normal of the tangent space, that is the the position vector. It also shouldn't have velocity in the orthogonal plane. This restricts the choice to a 1d subspace defined by the cross product of the position vector and the vectors spanning the ortogonal plane. It shouldn't have velocity in the orthogonal plane, because when rotating both points and tangent vectors are just multiplied by am matrix orthogonal A(t) that leaves the orthogonal plane fixed

If we have a point on a circle we get (r cos(w * t), r sin(w * t))
So the velocity is wr.
*/





/*
We have overlapping 2 shapes
We find a translation of one of the shapes, such that the shapes no longer overlap.
This is represented as a velocity vector of a geodesic such that after moving in that direction by the length of the vector the shapes no longer overlap.
It's probably best if this is a vector of least length that achives this.

We want to modify the velocity of the object along the normal such that after moving that frame
velocity along the normal is not positive, that is it's either zero or negative
the penetration is zero

Let p0 and p1 be the deepest penetrating points.
C(t) = dot(directionFromAToB(p0(t), p1(t)), normalAtP0) < 0.0f, 

directionFromAToB(a, b) = b - dot(b, a) * a;
C(t) = <p1(t) - <p1(t), p0(t)> * p0(t), n>
(<p1(t), p0(t)>)' = <p1'(t), p0(t)> + <p1(t), p0'(t)>
C'(t) = <p1'(t) - (<p1'(t), p0(t)> + <p1(t), p0'(t)>) * p0(t) - <p1(t), p0(t)> * p0'(t), n>
C' = 
<p1' - (<p1', p0> + <p1, p0'>) * p0 - <p1, p0> * p0', n> =
<p1', n> - <(<p1', p0> + <p1, p0'>) * p0, n> - <<p1, p0> * p0', n> = 
<p1', n> - (<p1', p0> + <p1, p0'>) <p0, n> - <p1, p0> <p0', n>
// If the points are close then <p1, p0> is near 1. If 
// <p1', p0> and <p1, p0'> should be close to zero0, because if the points are close then their tangent spaces are nearly equal and then one is a normal to the tangent space and the other vector is a tangent vector.
In that approximation we get

<p1', n> - <p0', n>

J = [ -n^T, n^T ]
V = [ p0', p1' ]
Under this approximation things kinda reduce to normal 3d case.

In spherical geometry object thar are large enough can't be moved apart.
*/
Arbiter::Arbiter(Body* b1, Body* b2) {
	if (b1 < b2) {
		body1 = b1;
		body2 = b2;
	} else {
		body1 = b2;
		body2 = b1;
	}

	numContacts = collide(contacts, body1, body2);
	friction = sqrtf(body1->friction * body2->friction);
}

void Arbiter::Update(Contact* newContacts, int numNewContacts) {
	Contact mergedContacts[2];

	for (i32 i = 0; i < numNewContacts; ++i) {
		Contact* cNew = newContacts + i;
		i32 k = -1;
		for (i32 j = 0; j < numContacts; ++j) {
			Contact* cOld = contacts + j;
			if (cNew->feature.value == cOld->feature.value) {
				k = j;
				break;
			}
		}

		if (k > -1) {
			Contact* c = mergedContacts + i;
			Contact* cOld = contacts + k;
			*c = *cNew;
			if (World::warmStarting) {
				c->Pn = cOld->Pn;
				c->Pt = cOld->Pt;
				c->Pnb = cOld->Pnb;
			} else {
				c->Pn = 0.0f;
				c->Pt = 0.0f;
				c->Pnb = 0.0f;
			}
		} else {
			mergedContacts[i] = newContacts[i];
		}
	}

	for (i32 i = 0; i < numNewContacts; i++) {
		contacts[i] = mergedContacts[i];
	}

	numContacts = numNewContacts;
}

void Arbiter::PreStep(f32 invDt) {
	const float k_allowedPenetration = 0.01f;
	//const float k_allowedPenetration = 0.001f;
	//float k_biasFactor = World::positionCorrection ? 0.2f : 0.0f;
	float k_biasFactor = World::positionCorrection ? 0.2f : 0.0f;

	for (int i = 0; i < numContacts; ++i)
	{
 		Contact* c = contacts + i;

		/*Vec4 r1 = c->position - body1->position;
		Vec4 r2 = c->position - body2->position;*/

		// Precompute normal mass, tangent mass, and bias.
		/*float rn1 = dot(r1, c->normal);
		float rn2 = dot(r2, c->normal);*/
		float kNormal = body1->invMass + body2->invMass;
		//kNormal += body1->invI * (Dot(r1, r1) - rn1 * rn1) + body2->invI * (Dot(r2, r2) - rn2 * rn2);
		c->massNormal = 1.0f / kNormal;

		//Vec2 tangent = Cross(c->normal, 1.0f);
		//float rt1 = Dot(r1, tangent);
		//float rt2 = Dot(r2, tangent);
		//float kTangent = body1->invMass + body2->invMass;
		//kTangent += body1->invI * (Dot(r1, r1) - rt1 * rt1) + body2->invI * (Dot(r2, r2) - rt2 * rt2);
		//c->massTangent = 1.0f / kTangent;

		//c->bias = -k_biasFactor * invDt * std::min(0.0f, c->separation + k_allowedPenetration);
		c->bias = 0.0f;

		World::accumulateImpulses = false;
		World::warmStarting = false;
		if (World::accumulateImpulses) {
			// Apply normal + friction impulse
			//Vec4 P = c->Pn * c->normal + c->Pt * tangent;
			Vec4 P = c->Pn * c->normal;

			body1->velocity -= body1->invMass * c->Pn * c->normalAToB;
			//body1->angularVelocity -= body1->invI * Cross(r1, P);

			body2->velocity += body2->invMass * c->Pn * c->normalBToA;
			//body2->angularVelocity += body2->invI * Cross(r2, P);
		}
	}
}
/*
In the basis (p, v, e0, e1), where 
p is the initial position, 
v is the initial velocity
e0, e1 is any basis of the space orthonormal to the first 2 vectors.

The positions of a point (x, y, z, w) expressed in this basis on a rotating body is
[cos(vt) -sin(vt) 0 0][x]
[sin(vt)  cos(vt) 0 0][y]  = r(t)
[ 0        0      1 0][z]
[ 0        0      0 1][w]

r'(t) =
[-xv sin(vt) - yv cos(wt)]
[ xv cos(vt) - yv sin(wt)]
[0]
[0]

r'(0) =
[-yv ]
[ xv ]
[0]
[0]

dot([x, y, z, w], [-yv, xv, 0, 0]) = -xyv + yxv = 0

*/
Vec4 velocityAtPoint(const Body& body, Vec4 point) {
	const auto rotationPlaneBasis0 = body.position;
	const auto rotationPlaneBasis1 = body.velocity;
	
	const auto t1 = dot(rotationPlaneBasis0, rotationPlaneBasis1);
	CHECK(t1 < 0.01f);
	//const auto orthogonalPlaneBasis = basisForOrthogonalComplement(rotationPlaneBasis0, rotationPlaneBasis1);

	const auto v = body.velocity.length();
	const auto p = Vec2(
		dot(point, rotationPlaneBasis0),
		dot(point, rotationPlaneBasis1)
		//dot(point, orthogonalPlaneBasis[0]),
		//dot(point, orthogonalPlaneBasis[1])
	);
	const auto t2 = p.length();

	const auto velocity = Vec4(
		-p.y * v,
		p.x * v,
		0.0f,
		0.0f
	);

	auto result =
		rotationPlaneBasis0 * velocity.x +
		rotationPlaneBasis1 * velocity.y;

	const auto t0 = dot(result, point);
	CHECK(t0 < 0.01f);
	return result;
} 
// Here is a formula using left contraction to get the velocity of a point. Without derivation.
// https://marctenbosch.com/ndphysics/NDrigidbody.pdf


//Vec4 velocityAtPoint(const Body& body, Vec4 point) {
//	const auto rotationPlaneBasis0 = body.position;
//	const auto rotationPlaneBasis1 = body.velocity;
//	const auto orthogonalPlaneBasis = basisForOrthogonalComplement(rotationPlaneBasis0, rotationPlaneBasis1);
//	const auto dist = distanceFromPlaneToPoint(Vec4(0.0f), orthogonalPlaneBasis[0], orthogonalPlaneBasis[1], point);
//	auto velocity = crossProduct(orthogonalPlaneBasis[0], orthogonalPlaneBasis[1], point);
//
//	{
//		const auto t1 = dot(rotationPlaneBasis0, orthogonalPlaneBasis[0]);
//		const auto t2 = dot(rotationPlaneBasis0, orthogonalPlaneBasis[1]);
//		const auto t3 = dot(rotationPlaneBasis1, orthogonalPlaneBasis[0]);
//		const auto t4 = dot(rotationPlaneBasis1, orthogonalPlaneBasis[1]);
//		const auto t = 0.0f;
//	}
//	/*if (dot(velocity, body.velocity) < 0.0f) {
//		velocity = -velocity;
//	}*/
//	// Not sure if the direction of the velocity is correct. + or -.
//	//auto velocity = crossProduct(orthogonalPlaneBasis[0], point, orthogonalPlaneBasis[1]);
//	velocity = velocity.normalized();
//	velocity *= dist;
//	velocity *= body.velocity.length();
//
//	if (dot(velocity, body.velocity) < 0.0f) {
//		velocity = -velocity;
//	}
//
//	// The length of body.velocity is the angular velocity velocity.
//
//	//const auto distanceFromPositionToOrtonormalPlane = 1.0f; // length(body.position).
//	//return body.velocity + velocity;
//	return velocity;
//}

void Arbiter::applyImpulse() {
	Body* b1 = body1;
	Body* b2 = body2;

	for (i32 i = 0; i < numContacts; i++) {
		Contact* c = contacts + i;
		/*c->r1 = c->position - b1->position;
		c->r2 = c->position - b2->position;*/

		// Relative velocity at contact
		/*Vec2 dv = b2->velocity + Cross(b2->angularVelocity, c->r2) - b1->velocity - Cross(b1->angularVelocity, c->r1);*/
		//Vec4 dv = b2->velocity - b1->velocity;
		/*const auto velocityAtPos2 = velocityAtPoint(*b2, c->position);
		const auto velocityAtPos1 = velocityAtPoint(*b1, c->position);*/
		const auto velocityAtPos2 = b2->velocity;
		const auto velocityAtPos1 = b1->velocity;
		if (velocityAtPos1.length() > 1.0f) {
			int x = 5;
		}

  		Vec4 relativeVelocityAtContactPosition = velocityAtPos2 - velocityAtPos1;
		const auto t0 = relativeVelocityAtContactPosition.length();
		// t0 Should be 2, because the 2 bodies start with velocity 1 and move straight into eachother.

		// Compute normal impulse
		//float vn = dot(dv, c->normal);
		float vn = dot(relativeVelocityAtContactPosition, c->normalAtPosition);
		if (vn > 5.0f) {
			int x = 5;
		}

		float dPn = c->massNormal * (-vn + c->bias);

		if (World::accumulateImpulses) {
			// Clamp the accumulated impulse
			float Pn0 = c->Pn;
			c->Pn = std::max(Pn0 + dPn, 0.0f);
			dPn = c->Pn - Pn0;
		} else {
			dPn = std::max(dPn, 0.0f);
		}

		// Apply contact impulse
		//Vec4 Pn = dPn * c->normal;

		//b1->velocity -= b1->invMass * Pn;
		b1->velocity -= b1->invMass * dPn * c->normalAToB;
		//b1->angularVelocity -= b1->invI * Cross(c->r1, Pn);

		//b2->velocity += b2->invMass * Pn;
		//b2->velocity += b2->invMass * Pn;
		b2->velocity += b2->invMass * dPn * c->normalBToA;
		//b2->angularVelocity += b2->invI * Cross(c->r2, Pn);

		// Relative velocity at contact
		//dv = b2->velocity + Cross(b2->angularVelocity, c->r2) - b1->velocity - Cross(b1->angularVelocity, c->r1);
		//dv = b2->velocity - b1->velocity;

		//Vec2 tangent = Cross(c->normal, 1.0f);
		//float vt = dot(dv, tangent);
		//float dPt = c->massTangent * (-vt);

		//if (World::accumulateImpulses)
		//{
		//	// Compute friction impulse
		//	float maxPt = friction * c->Pn;

		//	// Clamp friction
		//	float oldTangentImpulse = c->Pt;
		//	c->Pt = std::clamp(oldTangentImpulse + dPt, -maxPt, maxPt);
		//	dPt = c->Pt - oldTangentImpulse;
		//} else
		//{
		//	float maxPt = friction * dPn;
		//	dPt = std::clamp(dPt, -maxPt, maxPt);
		//}

		// Apply contact impulse
		//Vec4 Pt = dPt * tangent;

		//b1->velocity -= b1->invMass * Pt;
		//b1->angularVelocity -= b1->invI * Cross(c->r1, Pt);

		//b2->velocity += b2->invMass * Pt;
		//b2->angularVelocity += b2->invI * Cross(c->r2, Pt);
	}
}

//void Arbiter::PreStep(float inv_dt)
//{
//	const float k_allowedPenetration = 0.01f;
//	//float k_biasFactor = World::positionCorrection ? 0.2f : 0.0f;
//	float k_biasFactor = World::positionCorrection ? 0.5f : 0.0f;
//
//	for (int i = 0; i < numContacts; ++i)
//	{
//		Contact* c = contacts + i;
//
//		Vec4 r1 = c->position - body1->position;
//		Vec4 r2 = c->position - body2->position;
//
//		// Precompute normal mass, tangent mass, and bias.
//		float rn1 = dot(r1, c->normal);
//		float rn2 = dot(r2, c->normal);
//		float kNormal = body1->invMass + body2->invMass;
//		//kNormal += body1->invI * (Dot(r1, r1) - rn1 * rn1) + body2->invI * (Dot(r2, r2) - rn2 * rn2);
//		c->massNormal = 1.0f / kNormal;
//
//		//Vec2 tangent = Cross(c->normal, 1.0f);
//		//float rt1 = Dot(r1, tangent);
//		//float rt2 = Dot(r2, tangent);
//		//float kTangent = body1->invMass + body2->invMass;
//		//kTangent += body1->invI * (Dot(r1, r1) - rt1 * rt1) + body2->invI * (Dot(r2, r2) - rt2 * rt2);
//		//c->massTangent = 1.0f / kTangent;
//
//		c->bias = -k_biasFactor * inv_dt * std::min(0.0f, c->separation + k_allowedPenetration);
//
//		if (World::accumulateImpulses)
//		{
//			// Apply normal + friction impulse
//			//Vec4 P = c->Pn * c->normal + c->Pt * tangent;
//			Vec4 P = c->Pn * c->normal;
//
//			body1->velocity -= body1->invMass * P;
//			//body1->angularVelocity -= body1->invI * Cross(r1, P);
//
//			body2->velocity += body2->invMass * P;
//			//body2->angularVelocity += body2->invI * Cross(r2, P);
//		}
//	}
//}
//
//void Arbiter::ApplyImpulse()
//{
//	Body* b1 = body1;
//	Body* b2 = body2;
//
//	for (int i = 0; i < numContacts; ++i)
//	{
//		Contact* c = contacts + i;
//		c->r1 = c->position - b1->position;
//		c->r2 = c->position - b2->position;
//
//		// Relative velocity at contact
//		/*Vec2 dv = b2->velocity + Cross(b2->angularVelocity, c->r2) - b1->velocity - Cross(b1->angularVelocity, c->r1);*/
//		Vec4 dv = b2->velocity - b1->velocity;
//
//		// Compute normal impulse
//		float vn = dot(dv, c->normal);
//
//		float dPn = c->massNormal * (-vn + c->bias);
//
//		if (World::accumulateImpulses)
//		{
//			// Clamp the accumulated impulse
//			float Pn0 = c->Pn;
//			c->Pn = std::max(Pn0 + dPn, 0.0f);
//			dPn = c->Pn - Pn0;
//		} else
//		{
//			dPn = std::max(dPn, 0.0f);
//		}
//
//		// Apply contact impulse
//		Vec4 Pn = dPn * c->normal;
//
//		b1->velocity -= b1->invMass * Pn;
//		//b1->angularVelocity -= b1->invI * Cross(c->r1, Pn);
//
//		b2->velocity += b2->invMass * Pn;
//		//b2->angularVelocity += b2->invI * Cross(c->r2, Pn);
//
//		// Relative velocity at contact
//		//dv = b2->velocity + Cross(b2->angularVelocity, c->r2) - b1->velocity - Cross(b1->angularVelocity, c->r1);
//		dv = b2->velocity - b1->velocity;
//
//		//Vec2 tangent = Cross(c->normal, 1.0f);
//		//float vt = dot(dv, tangent);
//		//float dPt = c->massTangent * (-vt);
//
//		//if (World::accumulateImpulses)
//		//{
//		//	// Compute friction impulse
//		//	float maxPt = friction * c->Pn;
//
//		//	// Clamp friction
//		//	float oldTangentImpulse = c->Pt;
//		//	c->Pt = std::clamp(oldTangentImpulse + dPt, -maxPt, maxPt);
//		//	dPt = c->Pt - oldTangentImpulse;
//		//} else
//		//{
//		//	float maxPt = friction * dPn;
//		//	dPt = std::clamp(dPt, -maxPt, maxPt);
//		//}
//
//		// Apply contact impulse
//		//Vec4 Pt = dPt * tangent;
//
//		//b1->velocity -= b1->invMass * Pt;
//		//b1->angularVelocity -= b1->invI * Cross(c->r1, Pt);
//
//		//b2->velocity += b2->invMass * Pt;
//		//b2->angularVelocity += b2->invI * Cross(c->r2, Pt);
//	}
//}