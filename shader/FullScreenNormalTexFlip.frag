#version 410

smooth in vec2 texCoord;

uniform sampler2D tex;

out vec4 color;

void main()
{
	color = texture(tex, vec2(texCoord.x, 1 - texCoord.y));
	color.xyz = abs(color.xyz);
	color.w = 1.0;
}
