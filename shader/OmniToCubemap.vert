#version 410

layout(location = 0) in vec2 vPos;
layout(location = 2) in vec2 vTex;

uniform mat3 rotFaceToOmni;

smooth out vec3 omniDir;

void main()
{
	omniDir = rotFaceToOmni * vec3(vPos.x, vPos.y, -1.);

	//Flip y here so we write to the correct part of the texture.
	gl_Position = vec4(vPos.x, -vPos.y, 0., 1.);
}
