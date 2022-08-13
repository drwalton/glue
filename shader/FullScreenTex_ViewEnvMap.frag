#version 410

smooth in vec2 texCoord;

uniform mat3 invRotMat;

uniform samplerCube cubemap;

out vec4 color;

void main()
{
	//vec3 dir = vec3(texCoord.x*2 - 1, 1 - 2*texCoord.y, -1);
	vec3 dir = vec3(texCoord.x*2 - 1, 2*texCoord.y - 1, -1);
	vec3 lookupDir = invRotMat * dir;
	//lookupDir.x *= -1;

	//Convert to cubemap's left-handed coordinate space.
	lookupDir.z = -lookupDir.z;
	color = texture(cubemap, lookupDir);
}

