#ifndef GLUE_EXCEPTION_HPP_INCLUDED
#define GLUE_EXCEPTION_HPP_INCLUDED

#include <stdexcept>

namespace glue {

class FileException : public std::runtime_error
{
public:
	FileException(const std::string &what);
	virtual ~FileException() throw();
};

//!\brief Exception encountered when running OpenGL related code.
class GraphicsException : public std::runtime_error
{
public:
	GraphicsException(const std::string &what);
	virtual ~GraphicsException() throw();
};

//!\brief Exception encountered when compiling, linking or using an OpenGL
//!       shader.
class ShaderException : public GraphicsException
{
	public:
ShaderException(const std::string &what);
virtual ~ShaderException() throw();
};

#ifdef _DEBUG
#define throwOnGlError() \
	glue::throwOnGlError_("GL Error in file " __FILE__ ": ");
#else //_DEBUG
#define throwOnGlError() 
#endif //_DEBUG
void throwOnGlError_(const std::string &info);

}

#endif
