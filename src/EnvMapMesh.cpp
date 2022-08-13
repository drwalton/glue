#include "glue/EnvMapMesh.hpp"
#include <Eigen/Dense>

namespace glue {

EnvMapMesh::EnvMapMesh(
	const std::string &modelFilename, 
	glue::ShaderProgram * shader, 
	GLuint envMapTex)
	:envMapTex_(envMapTex),
	lodLevel_(5.f), 
	refractIdx_(0.8f)
{
	glue::ModelLoader loader(modelFilename);
	mesh_.fromModelLoader(loader);
	mesh_.shaderProgram(shader);
}

EnvMapMesh::~EnvMapMesh() throw()
{
}

void EnvMapMesh::camPosWorldSpace(const glue::vec3 & p)
{
	/*
	glue::vec4 p4(p.x(), p.y(), p.z(), 1.f);
	glue::mat4 worldToModel = modelToWorld().inverse();
	glue::vec4 cp4 = worldToModel * p4;
	for (size_t i = 0; i < 3; ++i) {
		camPosModelSpace_[i] = cp4[i] / cp4[3];
	}
	*/
	camPosModelSpace_ = p;
}

void EnvMapMesh::render()
{
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envMapTex_);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	mesh_.shaderProgram()->setUniform("cubemap", 0);
	mesh_.shaderProgram()->setUniform("lodLevel", lodLevel_);
	mesh_.shaderProgram()->setUniform("eta", refractIdx_);
	mesh_.shaderProgram()->setUniform("color", color_);
	mesh_.shaderProgram()->setUniform("camPosModelSpace", camPosModelSpace_);
	mesh_.modelToWorld(modelToWorld());
	mesh_.render();

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

}

