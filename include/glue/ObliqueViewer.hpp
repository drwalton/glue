#ifndef GLUE_OBLIQUEVIEWER_HPP_INCLUDED
#define GLUE_OBLIQUEVIEWER_HPP_INCLUDED

#include "Viewer.hpp"

namespace glue {

//!\brief A Viewer which renders a top-down plan view using an oblique projection.
class ObliqueViewer : public Viewer
{
public:
	ObliqueViewer(GLWindow *window);
	virtual ~ObliqueViewer() throw();
	virtual void processEvent(const SDL_Event &e);
	virtual void update();

	virtual void resize(size_t width, size_t height);
	virtual mat4 worldToCam() const;
private:
	glue::vec2 offset;
	float aspect;
	float scale;
	float moveSpeed;
	float scaleSpeed;
	void updateBlock();
};

}

#endif
