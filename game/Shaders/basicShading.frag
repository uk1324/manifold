#version 430 core

in vec3 normal; 
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


void main() {
//	vec3 lightDir = vec3(0, -1, 0);
//	float diffuse = max(0.0, dot(normal, -lightDir)) + 0.1;
//	fragColor = vec4(vec3(diffuse), 1.0);
	fragColor = vec4((normal + 1) / 2, 1.0);
    vec3 normalColor = (normal + 1) / 2;
    //float pattern = checkersTextureGradBox(uv * 4, dFdx(uv), dFdy(uv));
    float pattern = checkersTexture((uv) * 10);
	fragColor = vec4(normalColor * (pattern + 1.0 / 2.0), 1.0);
    //fragColor = vec4(uv, 0.0, 1.0);
    //fragColor = vec4(vec3(pattern), 1.0);
}
