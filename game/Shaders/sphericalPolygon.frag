#version 430 core

uniform vec4 cameraPos; 
uniform mat4 viewInverse4; 

in vec3 interpolatedNormal; 

in vec4 n0; 
in vec4 n1; 
in vec4 n2; 
in vec4 planeNormal; 
out vec4 fragColor;

/*generated end*/

#include "stereographic.glsl"

#include "shading.glsl"

in vec3 worldPos;

void main() {
	vec4 pos4 = inverseStereographicProjection(worldPos);
	if (dot(pos4, n0) < 0.0) discard;
	if (dot(pos4, n1) < 0.0) discard;
	if (dot(pos4, n2) < 0.0) discard;
	//fragColor = shade(worldPos, cameraPos, quatMultiply(quatInverseIfNormalized(cameraPos), pos4), planeNormal);
	// vertices get transformed by stereographicCamera.p and cameraPos = -inverseIfNormalized(stereographicCamera.p) so multipying by -cameraPos gives back the original position.
	fragColor = shade(worldPos, cameraPos, quatMultiply(-cameraPos, pos4), planeNormal);
	//fragColor = shade(worldPos, cameraPos, pos4, planeNormal);
}
