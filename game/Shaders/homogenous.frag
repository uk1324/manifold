#version 430 core

uniform vec2 screenSize; 
uniform mat4 inverseTransform; 

in vec4 n0; 
in vec4 n1; 
in vec4 n2; 
out vec4 fragColor;

/*generated end*/

//in vec3 worldPos;

uniform mat4 transform; 

#include "Utils/posFromDepth.glsl"
#include "stereographic.glsl"
#include "fog.glsl"

void main() {
	vec3 worldPos = posFromDepth(gl_FragCoord.xy / screenSize, gl_FragCoord.z, inverseTransform);
	vec4 pos4 = inverseStereographicProjection(worldPos);
	if (dot(pos4, n0) < 0.0) discard;
	if (dot(pos4, n1) < 0.0) discard;
	if (dot(pos4, n2) < 0.0) discard;

	//d = smoothstep(110.0, 10.0, d);
	fragColor = vec4(vec3(1.0, 1.0, 0.0) * fogScale(worldPos), 1.0);
	//fragColor = vec4((worldPos + 1.0) / 2.0, 1.0);
	//fragColor = vec4((pos4.xyz + 1.0) / 2.0, 1.0);
}
