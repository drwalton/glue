#ifndef GLUE_ANIMATEDMESH_HPP_INCLUDED
#define GLUE_ANIMATEDMESH_HPP_INCLUDED

#include "Mesh.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh, with a hook to
//!       add a function executed each time the mesh is rendered.
class AnimatedMesh : public Mesh
{
public:
	AnimatedMesh(const mat4 &modelToWorld = mat4::Identity());
	~AnimatedMesh() throw();
	
	void postRenderFunction(std::function<void(AnimatedMesh&)>);
	
	virtual void render();
	
private:
	std::function<void(AnimatedMesh&)> postRenderFunction_;
};

std::function <void(AnimatedMesh&)> growAnimatedMesh;

}

#endif
