#include "glue/FlyViewer.hpp"
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

FlyViewer::FlyViewer(GLWindow *window)
:Viewer(window), theta_(0.f), phi_(0.f),
thetaSpeed_(0.01f), phiSpeed_(0.01f),
translation_(vec3::Zero())
{
	int w, h;
	win_->size(&w, &h);
	perspective_ = perspective(fov, float(w)/float(h), zNear, zFar);
	updateTranslation();
	updateRotation();
	updateBuffer();
	throwOnGlError();
}

FlyViewer::~FlyViewer() throw()
{}

void FlyViewer::processEvent(const SDL_Event &event)
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
				int width = event.window.data1;
				int height = event.window.data2;
				resize(width, height);
			}
		}
	}
}

void FlyViewer::update()
{
	static const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if(keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_A] ||
		keystate[SDL_SCANCODE_S] || keystate[SDL_SCANCODE_D] ||
		keystate[SDL_SCANCODE_Q] || keystate[SDL_SCANCODE_E]) {
		vec4 translate(0.f, 0.f, 0.f, 1.f);
		float moveAmt = moveSpeed;
		if(keystate[SDL_SCANCODE_LSHIFT]) {
			moveAmt *= 2.f;
		}
		if(keystate[SDL_SCANCODE_W]) {
			translate.z() += moveAmt;
		}
		if(keystate[SDL_SCANCODE_A]) {
			translate.x() += moveAmt;
		}
		if(keystate[SDL_SCANCODE_D]) {
			translate.x() -= moveAmt;
		}
		if(keystate[SDL_SCANCODE_S]) {
			translate.z() -= moveAmt;
		}
		if(keystate[SDL_SCANCODE_Q]) {
			translate.y() -= moveAmt;
		}
		if(keystate[SDL_SCANCODE_E]) {
			translate.y() += moveAmt;
		}
		translate = rotate_.inverse() * translate;
		translation_ += vec3(translate.x()/translate.w(),
							 translate.y()/translate.w(), translate.z()/translate.w());
		updateTranslation();
		updateBuffer();
	}
}

mat3 FlyViewer::rotation() const
{
	return rotate_.block<3, 3>(0, 0);
}

vec3 FlyViewer::position() const
{
	vec4 pos = (translate_ * rotate_).inverse() * vec4(0.f, 0.f, 0.f, 1.f);
	return pos.block<3, 1>(0, 0);
}

void FlyViewer::rotation(float theta, float phi)
{
	theta_ = theta;
	phi_ = phi;
	updateRotation();
	updateBuffer();
}

void FlyViewer::position(const glue::vec3 & p)
{
	translation_ = p;
	updateTranslation();
	updateBuffer();
}

void FlyViewer::resize(size_t width, size_t height)
{
	float aspect = float(width)/float(height);
	perspective_ = perspective(fov, aspect, zNear, zFar);
	updateBlock();
	updateBuffer();
	glViewport(0, 0, width, height);
}

mat4 FlyViewer::worldToCam() const
{
	return rotate_ * translate_;
}

void FlyViewer::updateRotation()
{
	rotate_ =
	rotationAboutAxis(phi_, vec3(1.f, 0.f, 0.f)) *
	rotationAboutAxis(theta_, vec3(0.f, 1.f, 0.f));
	updateBlock();
}

void FlyViewer::updateTranslation()
{
	translate_ = translateMat4(translation_);
	updateBlock();
}

void FlyViewer::updateBlock()
{
	block_.worldToClip = perspective_ * rotate_ * translate_;
	block_.cameraPos = (rotate_ * translate_).inverse() * vec4(0.f, 0.f, 0.f, 1.f);
	block_.cameraDir = rotate_.inverse() * vec4(0.f, 0.f, -1.f, 1.f);
}

}
