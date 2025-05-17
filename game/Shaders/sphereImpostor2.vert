#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in vec3 instanceSphereCenter; 
layout(location = 6) in vec4 instanceSphereCenter4; 
layout(location = 7) in float instanceSphereRadius; 
layout(location = 8) in vec4 instanceN0; 
layout(location = 9) in vec4 instanceN1; 
layout(location = 10) in vec4 instanceN2; 
layout(location = 11) in vec4 instanceN3; 
layout(location = 12) in vec4 instancePlaneNormal; 

uniform mat4 transform; 

out vec3 worldPos; 
out vec3 rayDir; 

out vec3 sphereCenter; 
out vec4 sphereCenter4; 
out float sphereRadius; 
out vec4 n0; 
out vec4 n1; 
out vec4 n2; 
out vec4 n3; 
out vec4 planeNormal; 

void passToFragment() {
    sphereCenter = instanceSphereCenter; 
    sphereCenter4 = instanceSphereCenter4; 
    sphereRadius = instanceSphereRadius; 
    n0 = instanceN0; 
    n1 = instanceN1; 
    n2 = instanceN2; 
    n3 = instanceN3; 
    planeNormal = instancePlaneNormal; 
}

/*generated end*/
uniform vec3 cameraPos;
void main() {
    worldPos = (instanceTransform * vec4(vertexPosition, 1.0)).xyz;
    rayDir = worldPos - cameraPos;

    passToFragment();
    gl_Position = transform * (vec4(worldPos, 1.0));
}
