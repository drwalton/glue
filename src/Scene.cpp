#include "glue/Scene.hpp"

namespace glue {

void Scene::render()
{
	for(std::unique_ptr<Renderable> &r : renderableList) {
		r->render();
	}
}

}
