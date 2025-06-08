#version 430 core

in vec2 texturePosition; 
in vec2 atlasMin; 
in vec2 atlasMax; 

in vec3 color; 
out vec4 fragColor;

/*generated end*/

uniform sampler2D fontAtlas;

float sampleTexture(vec2 p) {
	float d = texture(fontAtlas, p).r;
	// Clamping creates edge bleeding.
	if (p.x < atlasMin.x || p.y < atlasMin.y || p.x > atlasMax.x || p.y > atlasMax.y) {
		d = 0.0;
	}
	return d;
}

void main() {
	vec2 p = texturePosition;
	float d = sampleTexture(p);
	float smoothing = fwidth(d) * 2.0;
	d -= 0.5 - smoothing;
	d = smoothstep(0.0, smoothing, d);

	fragColor = vec4(color, d);
	//fragColor = vec4(texturePosition, 0.0, 1.0);
}
