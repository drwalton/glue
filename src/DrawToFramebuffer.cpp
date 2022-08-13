#include "glue/DrawToFramebuffer.hpp"

namespace glue
{

void setFramebufferPixel(int x, int y, glue::vec3b value)
{
	glRasterPos2i(x, y);
	glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, value.data());
	glRasterPos2i(0, 0);
}

void drawSquareFramebuffer(int x, int y, int halfWidth, glue::vec3b value)
{
	int width = halfWidth*2 + 1;
	std::vector<glue::vec3b> buff(width*width);
	for(glue::vec3b &v : buff) {
		v = value;
	}
	
	glRasterPos2i(x-halfWidth, y-halfWidth);
	glDrawPixels(width, width, GL_RGB, GL_UNSIGNED_BYTE, value.data());
	glRasterPos2i(0, 0);
}

}
