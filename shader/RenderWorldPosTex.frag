
#version 410

smooth in vec3 worldPos;

out vec4 color;

void main()
{
	color = vec4(worldPos, 1);
}

