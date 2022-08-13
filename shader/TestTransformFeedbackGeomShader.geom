
#version 410

layout(points) in;
layout(points, max_vertices = 1) out;

in VertexData {
	float inVal;
} VertexOut[];

out float outVal;

void main()
{
	outVal = VertexOut[0].inVal;
	if(mod(outVal, 2.0) < .5) EmitVertex();
	EndPrimitive();
}

