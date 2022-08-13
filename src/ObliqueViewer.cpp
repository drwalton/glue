#include "glue/ObliqueViewer.hpp"
#include "glue/Matrices.hpp"
#include <iostream>

#include <cmath>

namespace glue {

#ifndef M_PI_4
#define M_PI_4 0.78539816339f
#endif

ObliqueViewer::ObliqueViewer(GLWindow * window)
	:Viewer(window),
	offset(0.f, 0.f), moveSpeed(0.005f), scaleSpeed(0.01f), aspect(1.f), scale(1.f)
{
	block_.cameraDir = vec4(0.f, -1.f, 0.f, 1.f);
	updateBlock();
	updateBuffer();
}

ObliqueViewer::~ObliqueViewer() throw()
{
}

void ObliqueViewer::processEvent(const SDL_Event & event)
{	
	if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_WINDOWEVENT) {
		if (event.window.windowID == win_->id()) {
			static int lastx = event.motion.x;
			static int lasty = event.motion.y;
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.motion.state & SDL_BUTTON(1)) {
					lastx = event.motion.x;
					lasty = event.motion.y;
				}
			}
			if (event.type == SDL_MOUSEMOTION) {
				if (event.motion.state & SDL_BUTTON(1)) {

					offset.x() += (event.motion.x - lastx) * moveSpeed;
					offset.y() -= (event.motion.y - lasty) * moveSpeed;
					lastx = event.motion.x;
					lasty = event.motion.y;
					updateBlock();
					updateBuffer();
				}
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
				int width = event.window.data1;
				int height = event.window.data2;
				resize(width, height);
			}
		}
	}

	if (event.type == SDL_MOUSEWHEEL) {
		float scrollAmt = scaleSpeed * event.wheel.y;
		scale += scrollAmt;
		if (scale < 0.f) scale = 0.f;
		updateBlock();
		updateBuffer();
	}
}

void ObliqueViewer::update()
{
}

void ObliqueViewer::resize(size_t width, size_t height)
{
	aspect = float(width)/float(height);
	updateBlock();
	updateBuffer();
}

mat4 ObliqueViewer::worldToCam() const
{
	mat4 m;
	m <<
		0.f, 0.f, 0.f, offset.x(),
		0.f, 0.f, 0.f, offset.y(),
		0.f, 0.f, 0.f, .5f,
		0.f, 0.f, 0.f, 1.f;
	return m;
}

void ObliqueViewer::updateBlock()
{
	std::cout << "Offset" << offset
		<< "\nScale " << scale
		<< "\nAspect " << aspect << std::endl;
	block_.cameraPos = vec4(offset.x(), 0.f, offset.y(), 1.f);
	block_.worldToClip <<
		scale / aspect, 0.f, 0.f, offset.x(),
		0.f, 0.f, -scale, offset.y(),
		0.f, -.1f, 0.f, .5f,
		0.f, 0.f, 0.f, 1.f;
}

}
