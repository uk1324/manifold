#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in vec2 instanceOffsetInAtlas; 
layout(location = 6) in vec2 instanceSizeInAtlas; 
layout(location = 7) in vec3 instanceColor; 

out vec2 texturePosition; 
out vec2 atlasMin; 
out vec2 atlasMax; 

out vec3 color; 

void passToFragment() {
    color = instanceColor; 
}

/*generated end*/

void main() {
	passToFragment();
	texturePosition = vertexPosition.xy;
	texturePosition += 1.0;
	texturePosition /= 2.0;
	texturePosition.y = 1.0 - texturePosition.y;
	texturePosition *= instanceSizeInAtlas;
	texturePosition += instanceOffsetInAtlas;
	atlasMin = instanceOffsetInAtlas;
	atlasMax = atlasMin + instanceSizeInAtlas;
//	vec2 center = (atlasMin + atlasMax) / 2.0;
//	float boxScale = 1.3;
//	texturePosition -= center;
//	texturePosition *= boxScale;
//	texturePosition += center;
	//gl_Position = vec4(instanceTransform * vec3(vertexPosition * boxScale, 1.0), 0.0, 1.0);
	gl_Position = instanceTransform * vec4(vertexPosition, 1.0);
}
