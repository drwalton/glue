#ifndef GLUE_CUBEMAPVIEWER_HPP_INCLUDED
#define GLUE_CUBEMAPVIEWER_HPP_INCLUDED

#include "GLWindow.hpp"
#include "VectorTypes.hpp"
#include "FullScreenQuad.hpp"
#include <memory>

namespace glue {

class ShaderProgram;

//!\brief Class for viewing a cubemap in an SDL window, as if from a viewpoint
//!       at the centre of the cubemap. Viewing direction can be changed by 
//!       dragging the mouse.
class CubemapViewer final {
public:
	CubemapViewer(GLWindow *win);
	~CubemapViewer();

	void processEvent(const SDL_Event &e);

	void draw(GLuint cubemap);

protected:
	CubemapViewer(const CubemapViewer&);
	CubemapViewer& operator=(const CubemapViewer&);
	GLWindow *win_;
	mat4 perspective_, rotation_;
	float theta_, phi_, thetaSpeed_, phiSpeed_;

	void updateRotation();
	void setUniforms(ShaderProgram *p);
	std::unique_ptr<ShaderProgram> shader_;
};

}

#endif
