#define _USE_MATH_DEFINES
#include "glue/SphericalPlot.hpp"
#include "glue/Directories.hpp"
#include "glue/ShaderProgram.hpp"

namespace glue
{

ShaderProgram *SphericalPlot::shader = nullptr;
size_t SphericalPlot::nInstances = 0;

SphericalPlot::~SphericalPlot() throw()
{
	--nInstances;
	if (nInstances == 0 && shader != nullptr) {
		delete shader;
		shader = nullptr;
	}
}

void SphericalPlot::render()
{
	vao.bind();
	shader->use();
	shader->setUniform("modelToWorld", modelToWorld());
	shader->setUniform("normToWorld", normToWorld());
	shader->setupCameraBlock();
    glDrawElements(GL_TRIANGLES, GLsizei(nElems_), GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_TRIANGLES, 0, GLsizei(sqrtNSamples_*sqrtNSamples_));
	shader->unuse();
	vao.unbind();
}

void SphericalPlot::makeShaderProgram()
{
	if (nInstances == 0 && shader == nullptr) {
		shader = new ShaderProgram({
			GLUE_SHADER_DIR + "SphericalPlot.vert",
			GLUE_SHADER_DIR + "SphericalPlot.frag"
		});
	} else {

	}
	++nInstances;
}

void SphericalPlot::makeElems(size_t sqrtNSamples, std::vector<GLuint>* elems)
{
	elems->reserve(sqrtNSamples * sqrtNSamples * 6);
	for (size_t v = 0; v < sqrtNSamples*(sqrtNSamples + 1); ++v) {
		unsigned i = v % sqrtNSamples;
		unsigned j = v / sqrtNSamples;
		if (i == sqrtNSamples || j == sqrtNSamples) continue;

		GLushort tlv, trv, blv, brv;
		tlv = i + j*sqrtNSamples;
		trv = i + 1 + j*sqrtNSamples;
		blv = i + (j + 1)*sqrtNSamples;
		brv = (i + 1) + (j + 1)*sqrtNSamples;

		elems->push_back(blv);
		elems->push_back(tlv);
		elems->push_back(trv);

		elems->push_back(trv);
		elems->push_back(brv);
		elems->push_back(blv);
	}
}

}
