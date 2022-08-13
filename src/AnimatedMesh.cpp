#include "glue/AnimatedMesh.hpp"

namespace glue {

AnimatedMesh::AnimatedMesh(const mat4 &modelToWorld)
	:Mesh(modelToWorld)
{}

AnimatedMesh::~AnimatedMesh() throw()
{}

void AnimatedMesh::postRenderFunction(std::function<void (AnimatedMesh &)> render)
{
	postRenderFunction_ = render;
}

void AnimatedMesh::render()
{
	Mesh::render();
	postRenderFunction_(*this);
}

}
