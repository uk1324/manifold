#include "Tri3d.hpp"

Vec3 triCenter(Vec3 v0, Vec3 v1, Vec3 v2) {
	return (v0 + v1 + v2) / 3.0f;
}

Vec3 triCenter(const Vec3* v) {
	return triCenter(v[0], v[2], v[1]);
}

//std::optional<RayTriIntersection> rayTriIntersection(Vec3 rayOrigin, Vec3 rayDirection, Vec3 v0, Vec3 v1, Vec3 v2) {
//	constexpr auto epsilon = std::numeric_limits<f32>::epsilon();
//
//    const auto edge1 = v1 - v0;
//    const auto edge2 = v2 - v0;
//    const auto ray_cross_e2 = cross(rayDirection, edge2);
//    const auto det = dot(edge1, ray_cross_e2);
//
//    if (det > -epsilon && det < epsilon) {
//        // ray parallel to triangle
//        return std::nullopt;    
//    }
//
//    const auto invDet = 1.0 / det;
//    const auto s = rayOrigin - v0;
//    const auto u = invDet * dot(s, ray_cross_e2);
//
//    if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u - 1) > epsilon)) {
//        return std::nullopt;
//    }
//        
//    const auto s_cross_e1 = cross(s, edge1);
//    const auto v = invDet * dot(rayDirection, s_cross_e1);
//
//    if ((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon)) {
//        return std::nullopt;
//    }
//
//    // At this stage we can compute t to find out where the intersection point is on the line.
//    const auto t = invDet * dot(edge2, s_cross_e1);
//
//    if (t > epsilon) {
//        // ray intersection
//        return rayOrigin + rayDirection * t;
//    } else {
//        // This means that there is a line intersection but not a ray intersection.
//        return std::nullopt;
//    }
//}

std::optional<RayTriIntersection> rayTriIntersection(Vec3 rayOrigin, Vec3 rayDirection, Vec3 v0, Vec3 v1, Vec3 v2) {
    constexpr auto epsilon = std::numeric_limits<f32>::epsilon();
    Vec3 v0v1 = v1 - v0;
    Vec3 v0v2 = v2 - v0;
    Vec3 pvec = cross(rayDirection, v0v2);
    float det = dot(v0v1, pvec);
#ifdef CULLING
    // If the determinant is negative, the triangle is back-facing.
    // If the determinant is close to 0, the ray misses the triangle.
    if (det < kEpsilon) return false;
#else
    // If det is close to 0, the ray and triangle are parallel.
    if (fabs(det) < epsilon) return std::nullopt;
#endif
    float invDet = 1 / det;

    Vec3 tvec = rayOrigin - v0;
    const auto u = dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1) return std::nullopt;

    Vec3 qvec = cross(tvec, v0v1);
    const auto v = dot(rayDirection, qvec) * invDet;
    if (v < 0 || u + v > 1) return std::nullopt;

    const auto t = dot(v0v2, qvec) * invDet;

    return RayTriIntersection{
        .position = rayOrigin + t * rayDirection,
        .barycentricCoordinates = Vec3(1.0f - u - v, u, v),
        .t = t
    };
}

std::optional<RayTriIntersection> rayTriIntersection(Vec3 rayOrigin, Vec3 rayDirection, const Vec3* v) {
    return rayTriIntersection(rayOrigin, rayDirection, v[0], v[1], v[2]);
}
