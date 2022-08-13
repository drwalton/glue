#ifndef GLUE_VIEWER_HPP_INCLUDED
#define GLUE_VIEWER_HPP_INCLUDED

#include "GLWindow.hpp"
#include "Constants.hpp"
#include "GLBuffer.hpp"

namespace glue {

//!\brief Abstract class encapsulating a 3D viewer, which interprets user input
//!       (as supplied by SDL) and uses this to move an OpenGL camera.
class Viewer
{
public:
	Viewer(GLWindow *window);
	virtual ~Viewer() throw();

	virtual void processEvent(const SDL_Event &e) = 0;
	virtual void update() = 0;

	void bindCameraBlock();

	virtual void resize(size_t width, size_t height) = 0;

	virtual mat4 worldToCam() const = 0;
protected:
	void updateBuffer();

	GLWindow *win_;
	UniformBuffer buffer_;
	CameraBlock block_;
};

}

#endif
