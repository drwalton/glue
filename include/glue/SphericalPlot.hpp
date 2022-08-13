#ifndef GLUE_SPHERICALPLOT_HPP_INCLUDED
#define GLUE_SPHERICALPLOT_HPP_INCLUDED

#include "glue/Renderable.hpp"
#include "glue/GLBuffer.hpp"
#include "glue/Constants.hpp"
#include "glue/VertexArrayObject.hpp"
#include <cfloat>

namespace glue
{

class ShaderProgram;

class SphericalPlot : public Renderable 
{
public:
	template <typename SphereFunc>
	SphericalPlot(SphereFunc f, size_t sqrtNSamples);
	~SphericalPlot() throw();

	template <typename SphereFunc>
	void replot(SphereFunc f);

	virtual void render();
private:
	void makeShaderProgram();
	template<typename SphereFunc>
	void makeMesh(SphereFunc f,
		size_t sqrtNSamples,
		std::vector<vec4>* vertices, std::vector<vec3> *normals);
	void makeElems(size_t sqrtNSamples,
		std::vector<GLuint> *elems);
	static size_t nInstances;
	static ShaderProgram *shader;
	size_t nElems_;
	VertexArrayObject vao;
	VertexBuffer verts, norms;
	ElementBuffer elems;
	size_t sqrtNSamples_;
};

}

#include "glue/impl/SphericalPlot.inl"

#endif
