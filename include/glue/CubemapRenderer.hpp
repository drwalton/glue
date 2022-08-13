#ifndef GLUE_CUBEMAPRENDERER_HPP_INCLUDED
#define GLUE_CUBEMAPRENDERER_HPP_INCLUDED

#include "Entity.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "FullScreenQuad.hpp"
#include <array>

namespace glue {

class UniformBuffer;

class CubemapRenderer : public Entity
{
public:
	enum class Format {RGBA8, RG32F};

	explicit CubemapRenderer(size_t facePixelWidth,
		Format format = Format::RGBA8);
	~CubemapRenderer() throw();

	//!\brief Render to cubemap by calling render() on a scene for each face
	//!  of the cube.
	//!\brief The supplied uniform buffer will be updated with suitable camera
	//!  matrices for each cube face.
	void render(Scene *scene, CameraBlockBuffer *camBuffer);

	//!\brief Render to cubemap by calling a custom render function for each face
	//!  of the cube.
	//!\brief The supplied uniform buffer will be updated with suitable camera
	//!  matrices for each cube face.
	void render(std::function<void(void)> renderFunc, CameraBlockBuffer *camBuffer);
	
	void render(std::function<void(const mat4&)> renderFunc);

	void loadExampleCubemap();

	//!\brief Render the content of a supplied omnidirectional image into the
	//! cubemap, using a simple approach (just rotate the image appropriately,
	//! and use the omnidirectional camera model.
	void addOmniImageSimple(Texture *omniIm, Texture *omniImMask, const mat4 &omniCamPose);

	void getImage(int face, void *buffer);
	
	void update(int face, void *buffer);

	GLuint texture();

	//!\brief Render the faces of the cubemap in turn to a renderbuffer.
	//!\note faces are rendered as a cube net, as follows:
	//!      | +y |
	//! | -x | -z | +x | +z |
	//!      | -y |
	void renderNet(Viewport v);

	//!\brief Save the ITM raycast results to files, for debugging.
	void saveRaycastToFile();
	
	static const std::array<mat4, 6> &cubemapRotations();
private:
	CubemapRenderer(const CubemapRenderer&);
	CubemapRenderer& operator=(const CubemapRenderer&);
	const Format format_;
	GLuint texture_, depthTexture_, framebuffer_, renderbuffer_;
	ShaderProgram drawIfNonZeroProgram_, drawTexProgram_, omniToCubemapProgram_;
	size_t facePixelWidth_;
	static const std::array<mat4, 6> cubemapRotations_;
};

}

#endif
