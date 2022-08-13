#ifndef GLUE_VERTEXARRAYOBJECT_HPP_INCLUDED
#define GLUE_VERTEXARRAYOBJECT_HPP_INCLUDED

#include <GL/glew.h>

namespace glue {

class VertexArrayObject final
{
public:
	VertexArrayObject();
	~VertexArrayObject() throw();

	void bind();
	void unbind();
private:
	GLuint vao_;
};

}

#endif
