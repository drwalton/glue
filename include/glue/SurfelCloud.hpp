#ifndef GLUE_SURFELCLOUD_HPP_INCLUDED
#define GLUE_SURFELCLOUD_HPP_INCLUDED

#include "Renderable.hpp"
#include "VertexArrayObject.hpp"
#include <memory>

namespace glue {

class ModelLoader;
class VertexBuffer;

class SurfelCloud : public Renderable
{
public:
	SurfelCloud(
		const std::vector<vec3> &verts,
		const std::vector<vec3> &norms,
		const std::vector<float> &radii);
	explicit SurfelCloud(const ModelLoader &loader, float radii);
	~SurfelCloud() throw();
	
	void shaderProgram(ShaderProgram *p);
	ShaderProgram *shaderProgram() const;
	
	virtual void render();

private:
	SurfelCloud(const SurfelCloud&);
	SurfelCloud &operator=(const SurfelCloud&);
	
	void verts(const std::vector<vec3> &verts);
	void norms(const std::vector<vec3> &norms);
	void radii(const std::vector<float> &norms);
	
	VertexArrayObject vao_;
	std::unique_ptr<VertexBuffer> verts_, norms_, radii_;
	size_t nVerts_;
	ShaderProgram *shader_;
};

}

#endif
