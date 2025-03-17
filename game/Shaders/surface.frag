#version 430 core

uniform float opacity; 

in vec3 interpolatedNormal; 
in vec2 uv; 
out vec4 fragColor;

/*generated end*/

float checkersTextureGradBox(vec2 p, vec2 ddx, vec2 ddy) {
    // filter kernel
    vec2 w = max(abs(ddx), abs(ddy)) + 0.01;  
    // analytical integral (box filter)
    vec2 i = 2.0*(abs(fract((p-0.5*w)/2.0)-0.5)-abs(fract((p+0.5*w)/2.0)-0.5))/w;
    // xor pattern
    return 0.5 - 0.5*i.x*i.y;                  
}

float checkersTexture( in vec2 p ) {
    vec2 q = floor(p);
    return mod( q.x+q.y, 2.0 );            // xor pattern
}

#include "oit.glsl"

void main() {
    vec3 normal = normalize(interpolatedNormal);
	fragColor = vec4((normal + 1) / 2, 1.0);
    vec3 normalColor = (normal + 1) / 2;
    float pattern = checkersTexture((uv) * 10);

    vec3 color = normalColor * (pattern + 1.0 / 2.0) * 0.5;

    //OIT_OUTPUT(color, opacity);
    fragColor = vec4(color, opacity);
}
