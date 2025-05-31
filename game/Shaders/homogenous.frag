#version 430 core

uniform vec2 screenSize; 
uniform mat4 inverseTransform; 
uniform vec4 cameraPos; 
uniform mat4 viewInverse4; 

in vec4 n0; 
in vec4 n1; 
in vec4 n2; 
in vec4 n3; 
in vec4 n4; 
in vec4 planeNormal; 
out vec4 fragColor;

/*generated end*/

//in vec3 worldPos;

uniform mat4 transform; 

#include "Utils/posFromDepth.glsl"
#include "stereographic.glsl"
#include "shading.glsl"

void main() {
	vec3 worldPos = posFromDepth(gl_FragCoord.xy / screenSize, gl_FragCoord.z, inverseTransform);
	vec4 pos4 = inverseStereographicProjection(worldPos);
	if (dot(pos4, n0) < 0.0) discard;
	if (dot(pos4, n1) < 0.0) discard;
	if (dot(pos4, n2) < 0.0) discard;
	if (dot(pos4, n3) < 0.0) discard;
	if (dot(pos4, n4) < 0.0) discard;
	//fragColor = shade(worldPos, cameraPos, quatMultiply(-cameraPos, pos4), planeNormal);
	fragColor = shade(worldPos, cameraPos, -viewInverse4 * pos4, planeNormal);
	//fragColor = shade(worldPos, cameraPos, pos4, planeNormal);
}
