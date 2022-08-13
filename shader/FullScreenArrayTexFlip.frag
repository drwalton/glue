#version 410

smooth in vec2 texCoord;

uniform sampler2DArray tex;
uniform int layer;

out vec4 color;

void main()
{
	color = texture(tex, vec3(texCoord.x, 1 - texCoord.y, float(layer)));
}
