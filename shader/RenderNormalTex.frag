
#version 410

smooth in vec3 normal;

out vec4 color;

void main()
{
	color = vec4(normal, 1);
}

