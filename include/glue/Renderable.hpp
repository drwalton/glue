#ifndef GLUE_RENDERABLE_HPP_INCLUDED
#define GLUE_RENDERABLE_HPP_INCLUDED

#include "Entity.hpp"
#include "ShaderProgram.hpp"

namespace glue {

//!\brief Abstract class encapsulating an Entity capable of being rendered.
class Renderable : public Entity
{
public:
	explicit Renderable(const mat4 &modelToWorld = mat4::Identity());

	virtual void render() = 0;
};

}

#endif
