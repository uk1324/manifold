#include "SurfaceCamera.hpp"

f32 SufaceCamera::normalSign() const {
    return normalFlipped ? -1.0f : 1.0f;
}

Vec3 SufaceCamera::cameraPosition(Vec3 position, Vec3 normalAtUvPosition) const {
    return position + height * normalAtUvPosition;
}
