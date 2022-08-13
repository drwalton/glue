
#version 410

layout(location = 0) in float input;

out float output;

void main()
{
	output = input + 1.f;
}

