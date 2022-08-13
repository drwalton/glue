#ifndef GLUE_CAMERAFRUSTUM_HPP_INCLUDED
#define GLUE_CAMERAFRUSTUM_HPP_INCLUDED

#include "ShaderProgram.hpp"
#include "GLBuffer.hpp"
#include "VertexArrayObject.hpp"
#include <memory>

namespace glue {
	
class ShaderProgram;

//!\brief A renderable object representing a camera frustum. Useful for
//!       visualising camera motion.
class CameraFrustum
{
public:
	virtual ~CameraFrustum() throw();
	static CameraFrustum &getInstance();

	void cameraTransform(const mat4 &t);
	void color(const vec4 &c);
	virtual void render(const Viewport &v);

private:
	explicit CameraFrustum();
	CameraFrustum(const CameraFrustum&);
	CameraFrustum &operator=(const CameraFrustum&);

	static std::unique_ptr<CameraFrustum> instancePtr_; 
	ShaderProgram shaderProgram_;
	VertexBuffer vertices_;
	vec4 color_;
	mat4 invCamTransform_;
	VertexArrayObject vao_;
};

}

#endif
