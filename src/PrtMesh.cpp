#include "glue/PrtMesh.hpp"
#include "glue/Constants.hpp"
#include "glue/Directories.hpp"
#include "glue/SphericalHarmonics.hpp"
#include <opencv2/opencv.hpp>

namespace glue {

std::unique_ptr<ShaderProgram> PrtMesh::mono4BandShader;
std::unique_ptr<ShaderProgram> PrtMesh::mono5BandShader;

std::unique_ptr<ShaderProgram> PrtMesh::shadow4BandShader;
std::unique_ptr<ShaderProgram> PrtMesh::shadow5BandShader;

std::unique_ptr<ShaderProgram> PrtMesh::shadow5BandShaderLdr;

std::unique_ptr<ShaderProgram> PrtMesh::albedo4BandShader;
std::unique_ptr<ShaderProgram> PrtMesh::albedo5BandShader;

std::unique_ptr<ShaderProgram> PrtMesh::color4BandShader;
std::unique_ptr<ShaderProgram> PrtMesh::color5BandShader;

std::unique_ptr<ShaderProgram> PrtMesh::colorShadow4BandShader;
std::unique_ptr<ShaderProgram> PrtMesh::colorShadow5BandShader;

size_t PrtMesh::nBands() const
{
	return calcNumSHBands(prtTexture_->nLayers());
}
size_t PrtMesh::nCoeffts() const
{
	return prtTexture_->nLayers();
}

void PrtMesh::realSurfaceAlbedo(const glue::vec3 &albedo)
{
	realSurfaceAlbedo_ = albedo;
}

PrtMesh::PrtMesh(
	Mode mode,
	const std::string &meshFilename, const std::string &prtCoefftsFilename,
	const std::string &shadowMeshFilename, const std::string &albedoTexFilename)
	:prtTexture_(new ArrayTexture(prtCoefftsFilename, getArrayTexMode(mode))),
	nShBands_(calcNumSHBands(prtTexture_->nLayers())),
	mode_(mode),
	realSurfaceAlbedo_(1.0f, 1.0f, 1.0f)
{
	ModelLoader loader(meshFilename);
	mesh_.fromModelLoader(loader);

	if (hasShadow()) {
		ModelLoader shadowModelLoader(shadowMeshFilename);
		shadowMesh_.fromModelLoader(shadowModelLoader);
	} else {
		shadowProgram_ = nullptr;
	}

	if (hasAlbedo()) {
		loadAlbedoTex(albedoTexFilename);
	}

	setupShader();
	throwOnGlError();
}

PrtMesh::~PrtMesh() throw()
{
}

void PrtMesh::setupShader()
{
	if(mode_ == MONO || mode_ == SHADOW_MONO || mode_ == SHADOW_MONO_LDR) {
		if(nShBands_ == 4) {
			if(!mono4BandShader) {
				mono4BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtMono4Band.frag"}));
			}
			shaderProgram_ = mono4BandShader.get();
		} else if(nShBands_ == 5) {
			if(!mono5BandShader) {
				mono5BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtMono5Band.frag"}));
			}
			shaderProgram_ = mono5BandShader.get();
		}
	} else if(mode_ == COLOR || mode_ == SHADOW_COLOR) {
		if(nShBands_ == 4) {
			if(!color4BandShader) {
				color4BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtColor4Band.frag"}));
			}
			shaderProgram_ = color4BandShader.get();
		} else if(nShBands_ == 5) {
			if(!color5BandShader) {
				color5BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtColor5Band.frag"}));
			}
			shaderProgram_ = color5BandShader.get();
		}
	} else if(mode_ == ALBEDO || mode_ == SHADOW_ALBEDO) {
		if(nShBands_ == 4) {
			if(!albedo4BandShader) {
				albedo4BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtAlbedoTex4Band.frag"}));
			}
			shaderProgram_ = albedo4BandShader.get();
		} else if(nShBands_ == 5) {
			if(!albedo5BandShader) {
				albedo5BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtAlbedoTex5Band.frag"}));
			}
			shaderProgram_ = albedo5BandShader.get();
		}
		shaderProgram_->setUniform("albedoTex", 1);
	} 
	mesh_.shaderProgram(shaderProgram_);
	shaderProgram_->setUniform("prtCoeffts", 0);

	if(mode_ == SHADOW_ALBEDO || mode_ == SHADOW_MONO) {
		if(nShBands_ == 4) {
			if(!shadow4BandShader) {
				shadow4BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtShadow4Band.frag"}));
			}
			shadowProgram_ = shadow4BandShader.get();
		} else if(nShBands_ == 5) {
			if(!shadow5BandShader) {
				shadow5BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtShadow5Band.frag"}));
			}
			shadowProgram_ = shadow5BandShader.get();
		}
		shadowMesh_.shaderProgram(shadowProgram_);
		shadowProgram_->setUniform("prtCoeffts", 0);
		shadowProgram_->setUniform("realSurfaceAlbedo", realSurfaceAlbedo_);
	}
	if (mode_ == SHADOW_MONO_LDR) {
		if (nShBands_ == 4) {
			throw std::runtime_error("Not implemented yet.");
		} else if (nShBands_ == 5) {
			if(!shadow5BandShaderLdr) {
				shadow5BandShaderLdr.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtShadow5BandLdr.frag"}));
			}
			shadowProgram_ = shadow5BandShaderLdr.get();
			shadowMesh_.shaderProgram(shadowProgram_);
			shadowProgram_->setUniform("prtCoeffts", 0);
			shadowProgram_->setUniform("realSurfaceAlbedo", realSurfaceAlbedo_);
		}
	}
	if (mode_ == SHADOW_COLOR) {
		if(nShBands_ == 4) {
			if(!colorShadow4BandShader) {
				colorShadow4BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtColorShadow4Band.frag"}));
			}
			shadowProgram_ = colorShadow4BandShader.get();
		} else if(nShBands_ == 5) {
			if(!colorShadow5BandShader) {
				colorShadow5BandShader.reset(new ShaderProgram(
					{glue::GLUE_SHADER_DIR + "PrtMono.vert",
					glue::GLUE_SHADER_DIR + "PrtColorShadow5Band.frag"}));
			}
			shadowProgram_ = colorShadow5BandShader.get();
		}
		shadowMesh_.shaderProgram(shadowProgram_);
		shadowProgram_->setUniform("prtCoeffts", 0);
	}
}

void PrtMesh::loadAlbedoTex(const std::string & albedoTexFilename)
{
	cv::Mat albedoTexImage = cv::imread(albedoTexFilename);
	cv::flip(albedoTexImage, albedoTexImage, 0);
	albedoTexture_.reset(new glue::Texture(GL_TEXTURE_2D, GL_RGB8, 
		albedoTexImage.cols, albedoTexImage.rows, 0, 
		GL_BGR, GL_UNSIGNED_BYTE, albedoTexImage.data));
}

ArrayTexture::PrtLoadMode PrtMesh::getArrayTexMode(Mode mode)
{
	if (mode == Mode::COLOR || mode == Mode::SHADOW_COLOR) {
		return ArrayTexture::PrtLoadMode::FLOAT_COLOR;
	} else {
		return ArrayTexture::PrtLoadMode::FLOAT;
	}
}

void PrtMesh::render()
{
	mesh_.modelToWorld(modelToWorld());

	if (hasShadow()) {
		shadowProgram_->setUniform("realSurfaceAlbedo", realSurfaceAlbedo_);
		shadowMesh_.modelToWorld(modelToWorld());
		if (mode_ == SHADOW_MONO_LDR) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
		} else {
			glDisable(GL_BLEND);
		}
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
		prtTexture_->bindToImageUnit(0);
		shadowMesh_.render();
		glEnable(GL_DEPTH_TEST);
		if (mode_ == SHADOW_MONO_LDR) {
			glDisable(GL_BLEND);
		}
	}
	prtTexture_->bindToImageUnit(0);
	if (hasAlbedo()) {
		albedoTexture_->bindToImageUnit(1);
	}

	mesh_.render();

}

void PrtMesh::lightingCoeffts(const SHProjection3 &lighting)
{
	shaderProgram_->setUniform("lightingCoeffts", lighting);
	if (hasShadow()) {
		shadowProgram_->setUniform("lightingCoeffts", lighting);
	}
}

void PrtMesh::monoColor(const glue::vec3 & color)
{
	if (hasMonoColor()) {
		shaderProgram_->setUniform("albedo", color);
	}
}

}
