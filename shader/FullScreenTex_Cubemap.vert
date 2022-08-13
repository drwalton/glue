#version 410

layout(location = 0) in vec2 vPos;
layout(location = 2) in vec2 vTex;

uniform mat3 rot;

smooth out vec3 texCoord;

void main()
{
	texCoord = rot * vec3(vPos, -1.f);
	//Convert to cubemap's left-handed coord space.
	texCoord.z = -texCoord.z;
	gl_Position = vec4(vPos, 0.f, 1.0f);
}
