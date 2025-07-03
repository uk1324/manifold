#version 430 core

in vec3 interpolatedNormal; 

in vec3 color; 
in mat4 model; 
out vec4 fragColor;

/*generated end*/

in vec3 worldPos;

void main() {
	vec3 normal = normalize(interpolatedNormal);
	float diffuse = dot(-vec3(0, 1, 0), normal);
	diffuse = max(0.0, diffuse);
	diffuse += 0.5;
	diffuse = clamp(diffuse, 0.0, 1.0);
	// diffuse = 1.0;
	float d = length(worldPos);
	float fadeDistanceStart = 10.0;
	float fadeDistanceEnd = 30.0;
	d = smoothstep(fadeDistanceEnd, fadeDistanceStart, d);
	fragColor = vec4(color * diffuse * d, 1.0);
	//fragColor = vec4(color, 1.0);
	//fragColor = vec4((normal + 1.0) / 2.0, 1.0);
}
