#version 410

smooth in vec2 texCoord;

out vec4 color;

void main()
{
	color = vec4(gl_FragCoord.x / 1000, gl_FragCoord.y / 1000, 0, 1);
}

