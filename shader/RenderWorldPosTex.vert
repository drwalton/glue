#version 410

layout(location = 0) in vec3 vPos;
layout(location = 2) in vec2 vTex;

smooth out vec3 worldPos;

void main()
{
	worldPos = vPos;
	gl_Position = vec4(vTex*2 - vec2(1,1), 0, 1);
}

