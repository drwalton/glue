
#version 410

layout(location = 0) in float input;

out float output;
out VertexData {
	float inVal;
} VertexOut;

void main()
{
	VertexOut.inVal = input;
}

