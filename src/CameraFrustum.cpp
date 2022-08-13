#include "glue/CameraFrustum.hpp"
#include "glue/Directories.hpp"
#include "glue/Constants.hpp"
#include "glue/GLWindow.hpp"
#include "glue/FullScreenQuad.hpp"
#include <Eigen/Dense>

namespace glue {

mat4 projectX, projectY, projectZ;

float frustumWidth = 0.1f;
float frustumLength = 0.2f;
float projectW = 2.f;

std::unique_ptr<CameraFrustum> CameraFrustum::instancePtr_(nullptr);

const std::vector<vec3> frustumTriangles{
	vec3(0.f, 0.f, 0.f),
	vec3(frustumWidth/2.f, frustumWidth/2.f, -frustumLength),
	vec3(-frustumWidth/2.f, frustumWidth/2.f, -frustumLength),

	vec3(0.f, 0.f, 0.f),
	vec3(frustumWidth/2.f, -frustumWidth/2.f, -frustumLength),
	vec3(-frustumWidth/2.f, -frustumWidth/2.f, -frustumLength),

	vec3(0.f, 0.f, 0.f),
	vec3(frustumWidth/2.f, frustumWidth/2.f, -frustumLength),
	vec3(frustumWidth/2.f, -frustumWidth/2.f, -frustumLength),

	vec3(0.f, 0.f, 0.f),
	vec3(-frustumWidth/2.f, frustumWidth/2.f, -frustumLength),
	vec3(-frustumWidth/2.f, -frustumWidth/2.f, -frustumLength),

	vec3(-frustumWidth/2.f, -frustumWidth/2.f, -frustumLength),
	vec3(-frustumWidth/2.f, frustumWidth/2.f, -frustumLength),
	vec3(frustumWidth/2.f, -frustumWidth/2.f, -frustumLength),

	vec3(frustumWidth/2.f, -frustumWidth/2.f, -frustumLength),
	vec3(-frustumWidth/2.f, frustumWidth/2.f, -frustumLength),
	vec3(frustumWidth/2.f, frustumWidth/2.f, -frustumLength),

	vec3(frustumWidth/2.f, frustumWidth/2.f, -frustumLength),
	vec3(-frustumWidth/2.f, frustumWidth/2.f, -frustumLength),
	vec3(0, frustumWidth, -frustumLength),
};

CameraFrustum::CameraFrustum()
	:shaderProgram_({
		GLUE_SHADER_DIR + "CameraFrustum.vert", 
		GLUE_SHADER_DIR + "CameraFrustum.frag"
	}),
	vertices_(frustumTriangles)
{
	projectX <<
		0.f, 0.f, 1.f / projectW, 0.f,
		0.f, 1.f / projectW, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f;
	projectY <<
		1.f / projectW, 0.f, 0.f, 0.f,
		0.f, 0.f, 1.f / projectW, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f;
	projectZ <<
		1.f / projectW, 0.f, 0.f, 0.f,
		0.f, 1.f / projectW, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f;
	vao_.bind();
	vertices_.bind();
	glEnableVertexAttribArray(UniformLocation::VERT);
	glVertexAttribPointer(UniformLocation::VERT, 3, GL_FLOAT, GL_FALSE, 0, 0);
	vertices_.unbind();
	vao_.unbind();
	throwOnGlError();
}

CameraFrustum::~CameraFrustum() throw()
{}

CameraFrustum &CameraFrustum::getInstance()
{
	if(instancePtr_.get() == nullptr) {
		instancePtr_.reset(new CameraFrustum());
	}
	return *instancePtr_;
}

void CameraFrustum::render(const Viewport &v)
{
	//Top left: along x axis
	vao_.bind();
	shaderProgram_.use();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	shaderProgram_.setUniform("invCamTransform", invCamTransform_);
	shaderProgram_.setUniform("color", color_);

	//Top left: along x axis
	shaderProgram_.setUniform("projectMat", projectX);
	glViewport(v.x, v.y + v.h / 2, v.w / 2, v.h / 2);
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(frustumTriangles.size()));

	//Top right: along y axis
	shaderProgram_.setUniform("projectMat", projectY);
	glViewport(v.x + v.w / 2, v.y + v.h / 2, v.w / 2, v.h / 2);
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(frustumTriangles.size()));

	//Bottom left: along z axis
	shaderProgram_.setUniform("projectMat", projectZ);
	glViewport(v.x, v.y, v.w / 2, v.h / 2);
	glDrawArrays(GL_TRIANGLES, 0, GLsizei(frustumTriangles.size()));

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	shaderProgram_.unuse();
	vao_.unbind();
}

void CameraFrustum::cameraTransform(const mat4 &t)
{
	this->invCamTransform_ = t.inverse();
}
void CameraFrustum::color(const vec4 &c)
{
	this->color_ = c;
}

}
