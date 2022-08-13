#include "glue/CubemapViewer.hpp"
#include "glue/Matrices.hpp"
#include "glue/ShaderProgram.hpp"
#include "glue/Directories.hpp"

#include <cmath>

namespace glue {

#ifndef M_PI_4
#define M_PI_4 0.78539816339f
#endif

CubemapViewer::CubemapViewer(GLWindow *win)
	:win_(win), theta_(0.f), phi_(0.f),
	thetaSpeed_(0.01f), phiSpeed_(0.01f)
{
	shader_.reset(new ShaderProgram(std::vector<std::string> {
		"CubemapViewer.vert",
		"CubemapViewer.frag"
	}));
	int w, h;
	win->size(&w, &h);
	perspective_ = perspective(M_PI_4, float(w)/float(h), 0.01f, 50.f);
	updateRotation();
}

CubemapViewer::~CubemapViewer()
{}

void CubemapViewer::processEvent(const SDL_Event & event)
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
				}
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
				//TODO resize events.
			}
		}
	}
}

void CubemapViewer::draw(GLuint cubemap)
{
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	glDisable(GL_DEPTH_TEST);

	setUniforms(shader_.get());
	FullScreenQuad::getInstance().render(*shader_);
}

void CubemapViewer::setUniforms(ShaderProgram *p)
{
	mat4 clipToWorld = (perspective_ * rotation_).inverse();
	p->use();
	p->setUniform("clipToWorld", clipToWorld);
	p->setUniform("cubemap", 0);
	p->unuse();
}

void CubemapViewer::updateRotation()
{
	rotation_ =
		rotationAboutAxis(phi_, vec3(1.f, 0.f, 0.f)) *
		rotationAboutAxis(theta_, vec3(0.f, 1.f, 0.f));
}

}
