#ifndef GLUE_ENVMAPMESH_HPP_INCLUDED
#define GLUE_ENVMAPMESH_HPP_INCLUDED

#include <glue/Mesh.hpp>

namespace glue {
	class ShaderProgram;
}

namespace glue
{

class EnvMapMesh final : public glue::Renderable
{
public:
	explicit EnvMapMesh(
		const std::string &modelFilename,
		glue::ShaderProgram *shader,
		GLuint envMapTex);
	~EnvMapMesh() throw();

	void camPosWorldSpace(const glue::vec3 &p);

	virtual void render();

	float lodLevel() const { return lodLevel_; };
	void lodLevel(float l) { lodLevel_ = l; };

	float refractIdx() const { return refractIdx_; };
	void refractIdx(float i) { refractIdx_ = i; };

	void color(const glue::vec3 &c) { color_ = c; }
private:
	glue::Mesh mesh_;
	GLuint envMapTex_;
	float lodLevel_;
	float refractIdx_;
	glue::vec3 color_;
	glue::vec3 camPosModelSpace_;
};

}

#endif
