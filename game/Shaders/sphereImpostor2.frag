#version 430 core

uniform vec3 cameraPos; 
uniform vec4 cameraPos4; 
uniform mat4 viewInverse4; 

in vec3 worldPos; 
in vec3 rayDir; 

in vec3 sphereCenter; 
in float sphereRadius; 
in vec4 n0; 
in vec4 n1; 
in vec4 n2; 
in vec4 n3; 
in vec4 planeNormal; 
out vec4 fragColor;

/*generated end*/

float sphIntersect(vec3 ro, vec3 rd, vec4 sph)
{
    vec3 oc = ro - sph.xyz;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - sph.w*sph.w;
    float h = b*b - c;
    if( h<0.0 ) return -1.0;
    h = sqrt( h );
    float l =  -b - h;
    if (l < 0.0) {
        return -b + h;
    }
    return l;
}

uniform mat4 transform;

#include "stereographic.glsl"
#include "shading.glsl"

void main() {
    vec3 ray = normalize(rayDir);
    float i = sphIntersect(cameraPos, ray, vec4(sphereCenter, sphereRadius));
    if (i < 0.0) {
        fragColor = vec4(0.5);
        //return;
        discard;
    }

    vec3 hitPos = ray * i + cameraPos;
    
	vec4 pos4 = inverseStereographicProjection(sphereCenter + normalize(hitPos - sphereCenter) * sphereRadius);

    vec4 clipPos = transform * vec4(hitPos, 1.0);
    gl_FragDepth = (clipPos.z / clipPos.w + 1.0) / 2.0;
	fragColor = vec4((normalize(hitPos - sphereCenter) + 1.0) / 2.0, 1.0);
    //fragColor = vec4(vec3(abs(i)), 1.0);
   // fragColor = vec4(worldPos, 1.0);
    //fragColor = vec4(cameraPos, 0.5);
    fragColor = shade(hitPos, cameraPos4, -viewInverse4 * pos4, planeNormal);
}
