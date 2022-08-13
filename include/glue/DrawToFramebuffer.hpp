#ifndef GLUE_DRAWTOFRAMEBUFFER_HPP_INCLUDED
#define GLUE_DRAWTOFRAMEBUFFER_HPP_INCLUDED

#include <glue/VectorTypes.hpp>

namespace glue
{

//!\brief Directly set the value of a single pixel of the currently active framebuffer.
void setFramebufferPixel(int x, int y, glue::vec3b value);

//!\brief Directly draw a square of the supplied width and color, centred around the 
//!       supplied pixel location in the currently active framebuffer.
void drawSquareFramebuffer(int x, int y, int halfWidth, glue::vec3b value);

}

#endif
