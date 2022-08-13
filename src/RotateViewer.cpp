#include "glue/RotateViewer.hpp"
#include "glue/Matrices.hpp"
#include <iostream>

#include <cmath>

namespace glue {

#ifndef M_PI_4
#define M_PI_4 0.78539816339f
#endif

const float zNear = 0.01f;
const float zFar = 50.f;
const float fov = M_PI_4;
const float moveSpeed = 0.1f;

RotateViewer::RotateViewer(GLWindow *window)
:Viewer(window), theta_(0.f), phi_(0.f),
thetaSpeed_(0.01f), phiSpeed_(0.01f),
delta_(-10.f), deltaSpeed_(0.1f)
{
	int w, h;
	win_->size(&w, &h);
	perspective_ = perspective(fov, float(w)/float(h), zNear, zFar);
	updateTranslation();
	updateRotation();
	updateBuffer();
	throwOnGlError();
}

RotateViewer::~RotateViewer() throw()
{}

void RotateViewer::processEvent(const SDL_Event &event)
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
					theta_ += (event.motion.x - lastx) * thetaSpeed_;
					phi_ += (event.motion.y - lasty) * phiSpeed_;
					lastx = event.motion.x;
					lasty = event.motion.y;
					updateRotation();
					updateBuffer();
				}
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
				//TODO resize events.
				int width = event.window.data1;
				int height = event.window.data2;
				resize(width, height);
			}
		}
	}
	if (event.type == SDL_MOUSEWHEEL) {
		float scrollAmt = deltaSpeed_ * event.wheel.y;
		delta_ += scrollAmt;
		if(delta_ > 0.f) delta_ = 0.f;
		updateTranslation();
		updateBuffer();
	}
}

mat3 RotateViewer::rotation() const
{
	return rotate_.block<3, 3>(0, 0);
}

vec3 RotateViewer::position() const
{
	vec4 pos = (translate_ * rotate_).inverse() * vec4(0.f, 0.f, 0.f, 1.f);
	return pos.block<3, 1>(0, 0);
}

void RotateViewer::distance(float dist)
{
	delta_ = -dist;
	updateTranslation();
	updateBuffer();
}

void RotateViewer::resize(size_t width, size_t height)
{
	float aspect = float(width)/float(height);
	perspective_ = perspective(fov, aspect, zNear, zFar);
	updateBlock();
	updateBuffer();
	glViewport(0, 0, width, height);
}

void RotateViewer::updateRotation()
{
	rotate_ =
		rotationAboutAxis(phi_, vec3(1.f, 0.f, 0.f)) *
		rotationAboutAxis(theta_, vec3(0.f, 1.f, 0.f));
	updateBlock();
}

void RotateViewer::updateTranslation()
{
	translate_ <<
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, delta_,
		0.f, 0.f, 0.f, 1.f;
	updateBlock();
}

void RotateViewer::updateBlock()
{
	block_.worldToClip = perspective_ *translate_ * rotate_;
	block_.cameraPos = (translate_ * rotate_).inverse() * vec4(0.f, 0.f, 0.f, 1.f);
	block_.cameraDir = rotate_.inverse() * vec4(0.f, 0.f, -1.f, 1.f);
}

mat4 RotateViewer::worldToCam() const
{
	return translate_ * rotate_;
}

}
