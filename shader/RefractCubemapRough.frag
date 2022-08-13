#version 410

in vec3 norm;
in vec3 pos;

uniform samplerCube cubemap;
uniform float eta;
uniform vec3 camPosModelSpace;

uniform float lodLevel;

out vec4 color;

void main()
{
	vec3 lookupDir = refract(
		normalize(pos - vec3(camPosModelSpace)), normalize(norm), eta);
	lookupDir.x = -lookupDir.x;
	color = textureLod(cubemap, lookupDir, lodLevel);
}

