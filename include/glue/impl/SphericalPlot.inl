#include "glue/SphericalHarmonics.hpp"

namespace glue
{

template <typename SphereFunc>
SphericalPlot::SphericalPlot(SphereFunc f, size_t sqrtNSamples)
	:verts(sqrtNSamples*(sqrtNSamples+1)*sizeof(vec4)),
	norms(sqrtNSamples*(sqrtNSamples+1)*sizeof(vec3)),
	elems(sqrtNSamples*sqrtNSamples*6*sizeof(GLuint)),
	sqrtNSamples_(sqrtNSamples)
{
	makeShaderProgram();
	std::vector<GLuint> elements;
	makeElems(sqrtNSamples, &elements);
	nElems_ = elements.size();
	elems.update(elements);
	replot(f);

	vao.bind();
	verts.bind();
	glEnableVertexAttribArray(UniformLocation::VERT);
	glVertexAttribPointer(UniformLocation::VERT, 4, GL_FLOAT, GL_FALSE, 0, 0);
	verts.unbind();
	norms.bind();
	glEnableVertexAttribArray(UniformLocation::NORM);
	glVertexAttribPointer(UniformLocation::NORM, 3, GL_FLOAT, GL_FALSE, 0, 0);
	norms.unbind();
	elems.bind();
	vao.unbind();
}

template <typename SphereFunc>
void SphericalPlot::replot(SphereFunc f)
{
	std::vector<vec4> vertices;
	std::vector<vec3> normals;
	makeMesh(f, sqrtNSamples_, &vertices, &normals);
	verts.update(vertices);
	norms.update(normals);
}

template<typename SphereFunc>
void SphericalPlot::makeMesh(SphereFunc f,
	size_t sqrtNSamples,
	std::vector<vec4>* vertices, std::vector<vec3> *normals)
{
	vertices->reserve(sqrtNSamples*(sqrtNSamples + 1));
	float sqrWidth = 1 / (float)sqrtNSamples;
	// Add vertex positions.
	for (unsigned i = 0; i < sqrtNSamples + 1; ++i) {
		for (unsigned j = 0; j < sqrtNSamples; ++j) {
			float u = (i * sqrWidth);
			float v = (j * sqrWidth);
			float theta = acos((2 * u) - 1);
			float phi = 2 * M_PI * v;
			vec3 dir3 = sphericalAngleToDir(theta, phi);
			vec4 dir(dir3.x(), dir3.y(), dir3.z(), 0.f);
			vec4 origin(0.0f, 0.0f, 0.0f, 1.0f);
			float val = f(theta, phi);
			vec4 vert = origin + dir*fabsf(val);
			vert.w() = val >= 0.f ? 1.f : -1.f;
	
			vertices->push_back(vert);
		}
	}
	normals->resize(vertices->size());
	//Make normals
	for (size_t v = 0; v < vertices->size(); ++v)
	{
		unsigned i = v % sqrtNSamples;
		unsigned j = v / sqrtNSamples;

		if (i == sqrtNSamples || j == sqrtNSamples) continue;

		/* Verts at each corner of a quad. */
		GLushort tlv, trv, blv, brv;
		tlv = i + j*sqrtNSamples;
		trv = i + 1 + j*sqrtNSamples;
		blv = i + (j + 1)*sqrtNSamples;
		brv = (i + 1) + (j + 1)*sqrtNSamples;

		/* Calculate norms. */
		vec4 across((*vertices)[trv] - (*vertices)[tlv]);
		vec4   down((*vertices)[blv] - (*vertices)[tlv]);
		vec3 across3 = across.block<3, 1>(0, 0);
		vec3 down3 = down.block<3, 1>(0, 0);
		vec3 norm = down3.cross(across3);
		if (fabsf(norm.x()) < FLT_EPSILON && fabsf(norm.y()) < FLT_EPSILON && fabsf(norm.z()) < FLT_EPSILON)
			(*normals)[tlv] = vec3(0.0f, 0.0f, 0.0f);
		else
			(*normals)[tlv] = norm.normalized();
	}
}
	
}
