#ifndef GLUE_FLYVIEWER_HPP_INCLUDED
#define GLUE_FLYVIEWER_HPP_INCLUDED

#include "Viewer.hpp"

namespace glue {

//!\brief A Viewer which
class FlyViewer : public Viewer
{
public:
	FlyViewer(GLWindow *window);
	virtual ~FlyViewer() throw();
	virtual void processEvent(const SDL_Event &e);
	virtual void update();
	
	mat3 rotation() const;
	vec3 position() const;

	void rotation(float theta, float phi);
	void position(const glue::vec3 &p);

	virtual void resize(size_t width, size_t height);
	virtual mat4 worldToCam() const;
private:
	float theta_, phi_, thetaSpeed_, phiSpeed_;
	vec3 translation_;
	mat4 translate_, rotate_, perspective_;
	void updateRotation();
	void updateTranslation();
	void updateBlock();
};

}

#endif
