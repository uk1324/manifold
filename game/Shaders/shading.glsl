float fogScale(vec3 pos) {
	float d = length(pos);
	float fogNear = 10.0;
	float fogFar = 20.0;
	d = (fogFar - d) / (fogFar - fogNear);
	return d;
}

//float d(vec4 normal4, vec4 lightDir) {
//	float diffuse = dot(-normalize(vec4(1, 1, 1, 1)), normal4);
//	diffuse = max(0, diffuse);
//	diffuse += 0.2;
//	diffuse = clamp(diffuse, 0, 1);
//}

vec4 quatInverseIfNormalized(vec4 v){ 
	return vec4(-v.xyz, v.w);
} 

//vec4 quatMultiply(vec4 l, vec4 r){ 
//	return vec4(l.w * r.xyz + r.w + l.xyz + cross(l.xyz, r.xyz), l.w * r.w - dot(l.xyz, r.xyz));
//} 

vec4 quatMultiply(vec4 l, vec4 r){ 
	return vec4(
        l.y * r.z - l.z * r.y + l.x * r.w + l.w * r.x,
		l.z * r.x - l.x * r.z + l.y * r.w + l.w * r.y,
		l.x * r.y - l.y * r.x + l.z * r.w + l.w * r.z,
		l.w * r.w - l.x * r.x - l.y * r.y - l.z * r.z
    );
} 


vec3 shade(vec4 lightPos, vec4 fragmentPos, vec4 fragmentNormal, vec4 cameraPos, vec3 lightColor) {
    //return vec3(0.2);
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    vec4 fromFragmentToLight = normalize(lightPos - fragmentPos);
    float diff = max(dot(fragmentNormal, fromFragmentToLight), 0.0);
    //float diff = abs(dot(fragmentNormal, fromFragmentToLight));
    vec3 diffuse = diff * lightColor;
  
  //return vec3(diffuse);
    // specular
    float specularStrength = 0.5;
    specularStrength = 0.0;
    vec4 viewDir = normalize(cameraPos - fragmentPos);
    vec4 reflectDir = reflect(-fromFragmentToLight, fragmentNormal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;  
    vec3 objectColor = vec3(1.0);
    vec3 result = (ambient + diffuse + specular) * objectColor;
    //result = vec3(fragmentNormal.xyz);
    result = vec3(diff);
//    result = vec3(distance(cameraPos, fragmentPos) / 4.0);
//    result = vec3(distance(vec4(1.0, 0.0, 0.0, 0.0), fragmentPos) / 4.0);
    // could attenuate based on geodesic distance
    return result;

//	vec4 fromFragmentToLight = normalize(lightPos - fragmentPos);
//	float diffuseScale = dot(fragmentNormal, fromFragmentToLight);
//	diffuseScale = max(0.0, diffuseScale);
//	float ambientScale = 0.2;
//	vec3 color = (diffuseScale + ambientScale) * lightColor;
//	//vec3 diffuse = lightColor * diffuseScale;
//	return color;
}

vec4 shade(vec3 worldPos, vec4 cameraPos4, vec4 pos4, vec4 normal4) {
    //cameraPos4 = -quatInverseIfNormalized(cameraPos4);
    //cameraPos4 = -cameraPos4;
    vec3 color = vec3(0.0);
//    color += shade(vec4(2.0, 0.0, 0.0, 0.0), pos4, normal4, cameraPos4, vec3(1.0, 0.0, 0.0));
//    color += shade(vec4(0.0, 2.0, 0.0, 0.0), pos4, normal4, cameraPos4, vec3(0.0, 1.0, 0.0));

    //color += shade(vec4(0.0, 2.0, 0.0, 0.0), pos4, -normal4, cameraPos4, vec3(1.0));
    color += shade(cameraPos4, pos4, normal4, cameraPos4, vec3(1.0));
    //color = vec3(pos4.xyz);
//    color += shade(vec4(0.0, 2.0, 0.0, 0.0), pos4, normal4, cameraPos4, vec3(0.0, 1.0, 0.0));
//    color += shade(vec4(0.0, 0.0, 2.0, 0.0), pos4, normal4, cameraPos4, vec3(0.0, 0.0, 1.0));
    //return vec4(pos4.xyz, 1.0);
    //return vec4(abs(normal4.xyz), 1.0);
	//color = abs(color);
    return vec4(color * fogScale(worldPos), 1.0);
//	float diffuse = dot(-normalize(vec4(1, 1, 1, 1)), normal4);
//	diffuse = max(0, diffuse);
//	diffuse += 0.2;
//	diffuse = clamp(diffuse, 0, 1);
	//diffuse = 1.0;

	//	fragColor = vec4(vec3(1.0, 1.0, 0.0) * diffuse * fogScale(worldPos), 1.0);
//	fragColor = vec4((worldPos + 1.0) / 2.0, 1.0);
//	fragColor = vec4((pos4.xyz + 1.0) / 2.0, 1.0);
	//fragColor = vec4(interpolatedColor.rgb, 1.0);
	//return vec4(vec3(1.0, 1.0, 0.0) * diffuse * fogScale(worldPos), 1.0);
}