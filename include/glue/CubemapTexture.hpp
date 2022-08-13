#ifndef GLUE_CUBEMAPTEXTURE_HPP_INCLUDED
#define GLUE_CUBEMAPTEXTURE_HPP_INCLUDED

#include "Texture.hpp"

namespace glue {

//!\brief Class encapsulating a (2D) OpenGL texture.
class CubemapTexture
{
public:
	explicit CubemapTexture(
    	GLenum target, GLenum internalFormat, GLsizei width,
    	GLsizei height, GLint border, GLenum format,
    	const GLvoid *data = nullptr);
	~CubemapTexture() throw();
	
	void update(GLenum face, void *data);

	void bindToImageUnit(GLuint unit);
	void unbind();
	
	Texture texture;
};

}

#endif
