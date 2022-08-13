#version 410

smooth in vec3 texCoord;

uniform samplerCube tex;

out vec4 color;

void main()
{
	color = texture(tex, texCoord);
}
