#version 410

in vec3 norm;
in vec3 pos;

uniform samplerCube cubemap;
uniform float lodLevel;
uniform vec3 camPosModelSpace;
uniform vec3 objColor;

out vec4 color;

void main()
{
	vec3 lookupDir = reflect(
		normalize(pos - vec3(camPosModelSpace)), normalize(norm));
	lookupDir.x = -lookupDir.x;
	color = textureLod(cubemap, lookupDir, lodLevel);
	color = vec4(objColor * color.xyz, 1.0);
}

