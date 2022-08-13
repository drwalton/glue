#version 410

smooth in vec2 texCoord;

uniform sampler2D tex;
uniform float brightnessOffset;

out vec4 color;

const vec3 norm1 = normalize(vec3(1,1,1));

void main()
{
	vec4 clr = texture(tex, texCoord);
	if(clr.xyz == vec3(0., 0., 0.)) discard;
	color = clr + vec4( 2 * brightnessOffset * norm1,0);
}

