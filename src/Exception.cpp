#include "glue/Exception.hpp"
#include <string>
#include <GL/glew.h>
#include "glue/ShaderProgram.hpp"

namespace glue {

FileException::FileException(const std::string &what)
:std::runtime_error(what.c_str())
{}

FileException::~FileException() throw()
{}

GraphicsException::GraphicsException(const std::string &what)
:std::runtime_error(what.c_str())
{}

GraphicsException::~GraphicsException() throw()
{}

ShaderException::ShaderException(const std::string &what)
:GraphicsException(what.c_str())
{}

ShaderException::~ShaderException() throw()
{}

void throwOnGlError_(const std::string &info)
{
	GLenum err = glGetError();
	std::string errname = glErrToString(err);
	if (err != GL_NO_ERROR) {
		throw GraphicsException(info + " " + errname);
	}
}

}
