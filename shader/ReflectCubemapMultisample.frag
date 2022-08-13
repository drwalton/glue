#version 410

in vec3 norm;
in vec3 pos;

uniform samplerCube cubemap;
uniform vec3 camPosModelSpace;
uniform float lodLevel;

out vec4 color;

void main()
{
	vec3 lookupDir = reflect(
		normalize(pos - vec3(camPosModelSpace)), normalize(norm));
	lookupDir.z = -lookupDir.z;
	vec3 perp1 = cross(lookupDir, vec3(0, 1, 0));
	vec3 perp2 = cross(lookupDir, perp1);

	float sampleDist = 0.01*lodLevel;

	color = texture(cubemap, lookupDir);
	color += texture(cubemap, lookupDir + sampleDist*perp1);
	color += texture(cubemap, lookupDir - sampleDist*perp1);
	color += texture(cubemap, lookupDir + sampleDist*perp2);
	color += texture(cubemap, lookupDir - sampleDist*perp2);
	color.xyz /= 5;
	color.w = 1;
}
