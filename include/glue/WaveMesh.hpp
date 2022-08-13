#ifndef GLUE_WAVEMESH_HPP_INCLUDED
#define GLUE_WAVEMESH_HPP_INCLUDED

#include "Renderable.hpp"
#include "GLBuffer.hpp"
#include "VertexArrayObject.hpp"
#include <GL/glew.h>
#include <memory>

namespace glue {

class WaveMesh : public Renderable
{
public:
	WaveMesh(const mat4 &modelToWorld = mat4::Identity());
	~WaveMesh() throw();

	virtual void render();
	
	void update();

	void shaderProgram(ShaderProgram *p);
	ShaderProgram* shaderProgram() const;
private:
	std::unique_ptr<VertexBuffer> vert_, norm_;
	std::unique_ptr<ElementBuffer> elem_;
	std::unique_ptr<ShaderProgram> renderProgram_, updateProgram_;
	VertexArrayObject vao_;
	size_t nElems_, nVerts_;
};

}

#endif
