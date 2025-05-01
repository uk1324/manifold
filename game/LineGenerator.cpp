#include "LineGenerator.hpp"
#include "Bezier.hpp"
#include <engine/Math/Interpolation.hpp>
#include <engine/Math/Angles.hpp>
#include <game/MeshUtils.hpp>

void LineGenerator::addLine(const std::vector<Vec3>& curvePoints) {
	if (curvePoints.size() < 4) {
		return;
	}
	//const auto offset = indices.size();
	const auto offset = vertexCount();

	const i32 ndivs = 8;
	const i32 ncurves = 1 + (curvePoints.size() - 4) / 3;
	Vec3 pts[4];
	std::unique_ptr<Vec3[]> P(new Vec3[(ndivs + 1) * ndivs * ncurves + 1]);
	std::unique_ptr<Vec3[]> N(new Vec3[(ndivs + 1) * ndivs * ncurves + 1]);
	std::unique_ptr<Vec2[]> st(new Vec2[(ndivs + 1) * ndivs * ncurves + 1]);
	for (uint32_t i = 0; i < ncurves; ++i) {
		for (uint32_t j = 0; j < ndivs; ++j) {
			Vec3 pts[]{
				curvePoints[i * 3],
				curvePoints[i * 3 + 1],
				curvePoints[i * 3 + 2],
				curvePoints[i * 3 + 3]
			};
			float s = j / (float)ndivs;
			Vec3 pt = evalBezierCurve(pts, s);
			Vec3 tangent = derivBezier(pts, s).normalized();
			bool swap = false;

			uint8_t maxAxis;
			if (std::abs(tangent.x) > std::abs(tangent.y))
				if (std::abs(tangent.x) > std::abs(tangent.z))
					maxAxis = 0;
				else
					maxAxis = 2;
			else if (std::abs(tangent.y) > std::abs(tangent.z))
				maxAxis = 1;
			else
				maxAxis = 2;

			Vec3 up, forward, right;

			switch (maxAxis) {
			case 0:
			case 1:
				up = tangent;
				forward = Vec3(0, 0, 1);
				right = cross(up, forward);
				forward = cross(right, up);
				break;
			case 2:
				up = tangent;
				right = Vec3(0, 0, 1);
				forward = cross(right, up);
				right = cross(up, forward);
				break;
			default:
				break;
			};

			up = up.normalized();
			forward = forward.normalized();
			right = right.normalized();
			float sNormalized = (i * ndivs + j) / float(ndivs * ncurves);
			//float rad = 0.1 * (1 - sNormalized);
			//float rad = 0.1 * (1 - sNormalized);
			float rad = 0.01f;
			//float rad = 0.04f;
			for (uint32_t k = 0; k <= ndivs; ++k) {
				float t = k / (float)ndivs;
				float theta = t * 2 * PI<f32>;
				Vec3 pc(cos(theta) * rad, 0, sin(theta) * rad);
				float x = pc.x * right.x + pc.y * up.x + pc.z * forward.x;
				float y = pc.x * right.y + pc.y * up.y + pc.z * forward.y;
				float z = pc.x * right.z + pc.y * up.z + pc.z * forward.z;
				P[i * (ndivs + 1) * ndivs + j * (ndivs + 1) + k] = Vec3(pt.x + x, pt.y + y, pt.z + z);
				N[i * (ndivs + 1) * ndivs + j * (ndivs + 1) + k] = Vec3(x, y, z).normalized();
				st[i * (ndivs + 1) * ndivs + j * (ndivs + 1) + k] = Vec2(sNormalized, t);
			}
		}
	}
	P[(ndivs + 1) * ndivs * ncurves] = curvePoints[curvePoints.size() - 1];
	N[(ndivs + 1) * ndivs * ncurves] = (curvePoints[curvePoints.size() - 2] - curvePoints[curvePoints.size() - 1]).normalized();
	st[(ndivs + 1) * ndivs * ncurves] = Vec2(1, 0.5);
	uint32_t numFaces = ndivs * ndivs * ncurves;
	std::unique_ptr<uint32_t[]> verts(new uint32_t[numFaces]);
	for (uint32_t i = 0; i < numFaces; ++i)
		verts[i] = (i < (numFaces - ndivs)) ? 4 : 3;
	std::unique_ptr<uint32_t[]> vertIndices(new uint32_t[ndivs * ndivs * ncurves * 4 + ndivs * 3]);
	uint32_t nf = 0, ix = 0;
	for (uint32_t k = 0; k < ncurves; ++k) {
		for (uint32_t j = 0; j < ndivs; ++j) {
			if (k == (ncurves - 1) && j == (ndivs - 1)) { break; }
			for (uint32_t i = 0; i < ndivs; ++i) {
				indicesAddQuad(
					indices,
					offset + nf,
					offset + nf + (ndivs + 1),
					offset + nf + (ndivs + 1) + 1,
					offset + nf + 1
				);
				/*vertIndices[ix] = nf;
				vertIndices[ix + 1] = nf + (ndivs + 1);
				vertIndices[ix + 2] = nf + (ndivs + 1) + 1;
				vertIndices[ix + 3] = nf + 1;*/
				ix += 4;
				++nf;
			}
			nf++;
		}
	}

	for (uint32_t i = 0; i < ndivs; ++i) {
		vertIndices[ix] = nf;
		vertIndices[ix + 1] = (ndivs + 1) * ndivs * ncurves;
		vertIndices[ix + 2] = nf + 1;
		ix += 3;
		nf++;
	}

	{
		const auto c = (ndivs + 1) * ndivs * ncurves + 1;
		for (i32 i = 0; i < c; i++) {
			positions.push_back(P[i]);
			normals.push_back(N[i]);
			//vertices.push_back(Vertex3Pnc{
			//	.position = P[i],
			//	.normal = N[i],
			//	//.color = Vec4(Color3::RED, 1.0f),
			//	.color = Vec4(Color3::fromHsv(f32(i) / f32(1000), 1.0f, 1.0f), 1.0f),
			//});
		}

		//for (i32 i = 0; i < ndivs * ndivs * ncurves * 4 + ndivs * 3; i++) {
		//	indices.push_back(vertIndices[i]);
		//}
	}
	
	//const auto c = (ndivs + 1) * ndivs * ncurves + 1;
	//ImGui::Text("vertex count: %d", c);
	//for (i32 i = 0; i < c; i++) {
	//	vertices.push_back(Vertex3Pnc{
	//		.position = P[i],
	//		.normal = N[i],
	//		//.color = Vec4(Color3::RED, 1.0f),
	//		.color = Vec4(Color3::fromHsv(f32(i) / f32(1000), 1.0f, 1.0f), 1.0f),
	//		});
	//}
}


void LineGenerator::addPlaneCurve(const std::vector<Vec3>& curvePoints, Vec3 planeNormal) {
	std::vector<Vec2> circlePoints;
	const i32 circlePointCount = 10;
	for (i32 i = 0; i < circlePointCount; i++) {
		const auto a = (f32(i) / f32(circlePointCount)) * TAU<f32>;
		circlePoints.push_back(Vec2::oriented(a));
	}

	const auto offset = vertexCount();

	planeNormal = planeNormal.normalized();
	for (i32 pointI = 0; pointI < curvePoints.size() - 1; pointI++) {
		const auto forward = (curvePoints[pointI + 1] - curvePoints[pointI]).normalized();
		const auto side = cross(forward, planeNormal).normalized();
		const auto circleCenter = curvePoints[pointI];

		f32 radius = 0.01f;
		for (i32 circlePointI = 0; circlePointI < circlePoints.size(); circlePointI++) {
			const auto& circlePoint = circlePoints[circlePointI];
			const auto normal = circlePoint.x * side + circlePoint.y * planeNormal;
			positions.push_back(circleCenter + normal * radius);
			normals.push_back(normal);
		}
	}

	const auto quadCount = circlePoints.size() * curvePoints.size();
	const auto triangleCount = 2 * quadCount;
	const auto vertexCount = 3 * triangleCount;

	for (i32 circleI = 0; circleI < curvePoints.size() - 2; circleI++) {
		i32 previousCirclePointI = circlePoints.size() - 1;
		for (i32 circlePointI = 0; circlePointI < circlePoints.size(); circlePointI++) {
			indicesAddQuad(
				indices,
				offset + circleI * circlePoints.size() + previousCirclePointI,
				offset + (circleI + 1) * circlePoints.size() + previousCirclePointI,
				offset + (circleI + 1) * circlePoints.size() + circlePointI,
				offset + (circleI) * circlePoints.size() + circlePointI
			);
			previousCirclePointI = circlePointI;
		}
	}
}

void LineGenerator::addFlatCurve(const std::vector<Vec3>& curvePoints, Vec3 cameraForward) {
	const auto offset = vertexCount();

	const auto normal = cameraForward.normalized();
	for (i32 pointI = 0; pointI < i32(curvePoints.size()) - 1; pointI++) {
		const auto forward = (curvePoints[pointI + 1] - curvePoints[pointI]).normalized();
		const auto side = cross(forward, normal).normalized();

		const auto width = 0.01f;
		const auto position = curvePoints[pointI];
		positions.push_back(position + side * width);
		positions.push_back(position - side * width);
		normals.push_back(cameraForward);
		normals.push_back(cameraForward);
	}

	for (i32 i = 0; i < i32(curvePoints.size()) - 2; i++) {
		indicesAddQuad(
			indices,
			offset + i * 2,
			offset + (i + 1) * 2,
			offset + (i + 1) * 2 + 1,
			offset + i * 2 + 1
		);
	}
}

void LineGenerator::addCircularArc(Vec3 a, Vec3 b, Vec3 circleCenter, f32 tubeRadius) {
	const auto p0 = a - circleCenter;
	const auto p1 = b - circleCenter;
	const auto p0Length = p0.length();
	const auto p1Length = p1.length();
	const auto angleBetween = acos(std::clamp(dot(p0, p1) / p0Length / p1Length, -1.0f, 1.0f));
	const auto u0 = p0 / p0Length;

	// Gram-Schmidt orthogonalization.
	const auto velocityOutOfP0 = (p1 - dot(p1, u0) * u0).normalized() * p0Length;
	if (velocityOutOfP0.length() == 0) {
		// antipodal or eqals points because p0, p1 colinear
		return;
	}
	addCircularArc(p0, velocityOutOfP0, circleCenter, angleBetween, tubeRadius);
}

void LineGenerator::addCircularArc(Vec3 aRelativeToCenter, Vec3 velocityOutOfA, Vec3 circleCenter, f32 arclength, f32 tubeRadius) {

	velocityOutOfA = velocityOutOfA.normalized() * aRelativeToCenter.length();
	const auto t0 = aRelativeToCenter.length();
	const auto t1 = velocityOutOfA.length();

	std::vector<Vec2> circlePoints;
	const i32 circlePointCount = 10;
	for (i32 i = 0; i < circlePointCount; i++) {
		const auto a = (f32(i) / f32(circlePointCount)) * TAU<f32>;
		circlePoints.push_back(Vec2::oriented(a));
	}

	const auto offset = vertexCount();

	const auto count = 100;
	const auto curveBinormal = cross(aRelativeToCenter, velocityOutOfA).normalized();
	for (i32 curveI = 0; curveI < count; curveI++) {
		const auto curveAngle = lerp(0.0f, arclength, f32(curveI) / f32(count - 1));
		const auto c = cos(curveAngle);
		const auto s = sin(curveAngle);
		auto curveNormal = c * aRelativeToCenter + s * velocityOutOfA;
		const auto curvePosition = circleCenter + curveNormal;
		curveNormal = curveNormal.normalized();
		for (const auto& circlePoint : circlePoints) {
			const auto surfaceNormal = 
				tubeRadius * 
				(circlePoint.x * curveNormal + circlePoint.y * curveBinormal);
			const auto surfacePosition = curvePosition + surfaceNormal;
			positions.push_back(surfacePosition);
			normals.push_back(surfaceNormal);
		}
		for (i32 circleI = 0; circleI < circlePointCount; circleI++) {
			
		}
	}

	for (i32 circleI = 0; circleI < count - 1; circleI++) {
		i32 previousCirclePointI = circlePoints.size() - 1;
		for (i32 circlePointI = 0; circlePointI < circlePoints.size(); circlePointI++) {
			indicesAddQuad(
				indices,
				offset + circleI * circlePoints.size() + previousCirclePointI,
				offset + (circleI + 1) * circlePoints.size() + previousCirclePointI,
				offset + (circleI + 1) * circlePoints.size() + circlePointI,
				offset + (circleI) * circlePoints.size() + circlePointI
			);
			previousCirclePointI = circlePointI;
		}
	}
}

void LineGenerator::reset() {
	positions.clear();
	normals.clear();
	uvs.clear();
	indices.clear();
}

i32 LineGenerator::vertexCount() const{
	return i32(positions.size());
}
