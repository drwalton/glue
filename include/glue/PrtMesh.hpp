#ifndef GLUE_PRTMESH_HPP_INCLUDED
#define GLUE_PRTMESH_HPP_INCLUDED

#include "glue/Mesh.hpp"
#include "glue/PrtCoeffts.hpp"
#include "glue/Texture.hpp"
#include "glue/ArrayTexture.hpp"
#include "glue/ShaderProgram.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh.
class PrtMesh : public Renderable
{
public:
	virtual ~PrtMesh() throw();

	virtual void render();
	
	void lightingCoeffts(const SHProjection3 &lighting);
	
	void monoColor(const glue::vec3 &color);

	bool hasShadow() const {
		return mode_ == SHADOW_ALBEDO || mode_ == SHADOW_MONO || mode_ == SHADOW_COLOR || mode_ == SHADOW_MONO_LDR;
	}

	bool hasAlbedo() const {
		return mode_ == SHADOW_ALBEDO || mode_ == ALBEDO;
	}

	bool hasMonoColor() const {
		return mode_ == MONO || mode_ == SHADOW_MONO || mode_ == SHADOW_MONO_LDR;
	}

	size_t nBands() const;
	size_t nCoeffts() const;
	
	void realSurfaceAlbedo(const vec3 &albedo);
protected:
	enum Mode {MONO, SHADOW_ALBEDO, SHADOW_MONO, SHADOW_MONO_LDR, ALBEDO, COLOR, SHADOW_COLOR};
	PrtMesh(Mode mode,
		const std::string &meshFilename,
		const std::string &prtCoefftsFilename,
		const std::string &shadowMeshFilename,
		const std::string &albedoTexFilename);
private:
	PrtMesh(const PrtMesh &);
	PrtMesh &operator=(const PrtMesh &);
	
	void setupShader();
	
	Mesh mesh_, shadowMesh_;
	std::unique_ptr<ArrayTexture> prtTexture_;
	std::unique_ptr<Texture> albedoTexture_;
	
	static std::unique_ptr<ShaderProgram> mono4BandShader;
	static std::unique_ptr<ShaderProgram> mono5BandShader;
	
	static std::unique_ptr<ShaderProgram> shadow4BandShader;
	static std::unique_ptr<ShaderProgram> shadow5BandShader;

	static std::unique_ptr<ShaderProgram> shadow5BandShaderLdr;
	
	static std::unique_ptr<ShaderProgram> albedo4BandShader;
	static std::unique_ptr<ShaderProgram> albedo5BandShader;

	static std::unique_ptr<ShaderProgram> color4BandShader;
	static std::unique_ptr<ShaderProgram> color5BandShader;

	static std::unique_ptr<ShaderProgram> colorShadow4BandShader;
	static std::unique_ptr<ShaderProgram> colorShadow5BandShader;
	
	size_t nShBands_;
	Mode mode_;
	glue::vec3 realSurfaceAlbedo_;
	ShaderProgram *shaderProgram_, *shadowProgram_;

	void loadAlbedoTex(const std::string &albedoTexFilename);
	static ArrayTexture::PrtLoadMode getArrayTexMode(Mode mode);
};

}

#endif
