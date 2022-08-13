#ifndef GLUE_SCENE_HPP_INCLUDED
#define GLUE_SCENE_HPP_INCLUDED

#include "Renderable.hpp"
#include <vector>
#include <memory>

namespace glue {

class Scene final {
public:
	void render();
	std::vector<std::unique_ptr<Renderable> > renderableList;
};

}

#endif
