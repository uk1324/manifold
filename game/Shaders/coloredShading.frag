#version 430 core

in vec3 interpolatedNormal; 
in vec4 interpolatedColor; 
out vec4 fragColor;

/*generated end*/

void main() {
	vec3 normal = normalize(interpolatedNormal);
	float diffuse = dot(-vec3(0, 1, 0), normal);
	diffuse = max(0, diffuse);
	diffuse += 0.5;
	diffuse = clamp(diffuse, 0, 1);
	//fragColor = vec4(interpolatedColor * diffuse, 1.0);
	fragColor = vec4(interpolatedColor.rgb * diffuse, 1.0);
}
