#version 410

smooth in vec2 texCoord;

uniform sampler2D tex;

out vec4 color;

void main()
{
	float gray = texture(tex, vec2(texCoord.x, 1 - texCoord.y)).r;
	color = vec4(gray, gray, gray, 1);
}
