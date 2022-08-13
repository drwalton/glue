#include "glue/SurfelCloud.hpp"
#include "glue/Constants.hpp"
#include "glue/ModelLoader.hpp"
#include "glue/GLBuffer.hpp"

namespace glue {


SurfelCloud::SurfelCloud(
	const std::vector<vec3> &v, const std::vector<vec3> &n, const std::vector<float> &r)
	:shader_(nullptr)
{
	verts(v);
	norms(n);
	radii(r);
}

SurfelCloud::SurfelCloud(
	const ModelLoader &loader, float radius)
	:shader_(nullptr)
{
	verts(loader.vertices());
	if(loader.hasNormals()) {
		norms(loader.normals());
	}
	std::vector<float> r;
	r.resize(loader.vertices().size());
	for(float &f : r) {
		f = radius;
	}
	radii(r);
}

SurfelCloud::~SurfelCloud() throw()
{}

void SurfelCloud::verts(const std::vector<vec3> &v)
{
	verts_.reset(new VertexBuffer(v, GL_DYNAMIC_DRAW));
	vao_.bind();
	verts_->bind();
	glEnableVertexAttribArray(UniformLocation::VERT);
	glVertexAttribPointer(UniformLocation::VERT, 3, GL_FLOAT, GL_FALSE, 0, 0);
	verts_->unbind();
	vao_.unbind();
	nVerts_ = v.size();
}

void SurfelCloud::norms(const std::vector<vec3> &n)
{
	norms_.reset(new VertexBuffer(n, GL_DYNAMIC_DRAW));
	vao_.bind();
	norms_->bind();
	glEnableVertexAttribArray(UniformLocation::NORM);
	glVertexAttribPointer(UniformLocation::NORM, 3, GL_FLOAT, GL_FALSE, 0, 0);
	norms_->unbind();
	vao_.unbind();
}

void SurfelCloud::radii(const std::vector<float> &r)
{
	radii_.reset(new VertexBuffer(r, GL_DYNAMIC_DRAW));
	vao_.bind();
	radii_->bind();
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	radii_->unbind();
	vao_.unbind();
}

void SurfelCloud::shaderProgram(ShaderProgram *p)
{
	shader_ = p;
}

ShaderProgram *SurfelCloud::shaderProgram() const
{
	return shader_;
}

void SurfelCloud::render()
{
	if(shader_ == nullptr) {
		throw GraphicsException("Attempted to render mesh without supplying shader program.");
	}
	vao_.bind();
	shader_->use();
	shader_->setUniform("modelToWorld", modelToWorld());
	shader_->setUniform("normToWorld", normToWorld());
	shader_->setupCameraBlock();
	glDrawArrays(GL_POINTS, 0, GLsizei(nVerts_));
	shader_->unuse();
	vao_.unbind();
}

}
