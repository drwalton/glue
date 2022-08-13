#version 410

smooth in float depth;

out float fDepth;

void main()
{
	fDepth = depth;
}

