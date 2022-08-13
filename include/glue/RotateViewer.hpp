#ifndef GLUE_ROTATEVIEWER_HPP_INCLUDED
#define GLUE_ROTATEVIEWER_HPP_INCLUDED

#include "Viewer.hpp"

namespace glue {

//!\brief A Viewer which rotates about the origin.
class RotateViewer : public Viewer
{
public:
	RotateViewer(GLWindow *window);
	virtual ~RotateViewer() throw();
	virtual void processEvent(const SDL_Event &e);
	virtual void update() {};
	mat3 rotation() const;
	vec3 position() const;
	void distance(float dist);
	virtual void resize(size_t width, size_t height);
	virtual mat4 worldToCam() const;
private:
	float theta_, phi_, thetaSpeed_, phiSpeed_;
	float delta_, deltaSpeed_;
	mat4 translate_, rotate_, perspective_;
	void updateRotation();
	void updateTranslation();
	void updateBlock();
};

}

#endif
