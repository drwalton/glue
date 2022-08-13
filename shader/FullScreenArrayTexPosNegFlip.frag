#version 410

smooth in vec2 texCoord;

uniform sampler2DArray tex;
uniform int layer;

out vec4 color;

void main()
{
	vec4 s = texture(tex, vec3(texCoord.x, 1 - texCoord.y, float(layer)));
	color = (s.x >= 0.0) ? vec4(0, s.x, 0, 1) : vec4(0, 0, -s.x, 1);
}
