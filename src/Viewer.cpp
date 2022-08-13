#include "glue/Viewer.hpp"
#include "glue/Matrices.hpp"
#include <iostream>

#include <cmath>

namespace glue {

#ifndef M_PI_4
#define M_PI_4 0.78539816339f
#endif

Viewer::Viewer(GLWindow *window)
:win_(window), buffer_(&block_, sizeof(CameraBlock))
{
	bindCameraBlock();
}

Viewer::~Viewer() throw()
{}

void Viewer::bindCameraBlock()
{
	buffer_.bindRange(UniformBlock::CAMERA);
}

void Viewer::updateBuffer()
{
	buffer_.update(&block_);
}

}
