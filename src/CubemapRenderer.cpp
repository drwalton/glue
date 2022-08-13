#include "glue/CubemapRenderer.hpp"
#include "glue/GLBuffer.hpp"
#include "glue/Constants.hpp"
#include "glue/Matrices.hpp"
#include "glue/Directories.hpp"
#include <Eigen/Dense>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace glue {

const mat4 cubemapPerspective =
	(mat4() <<
	1.f, 0.f, 0.f, 0.f,
	0.f, -1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
	).finished() *
	perspective(float(M_PI_2), 1.f, 0.01f, 50.f);



//!\brief View transforms corresponding to each face of the cubemap.
//!  These are camera transforms, and should be used when rendering world-space
//! objects into the cubemap
//!  Each rotation moves from the camera space of the negative z face to that
//! of the current face.
const std::array<mat4, 6> CubemapRenderer::cubemapRotations_  = {{
	angleAxisMat4(float(-M_PI_2), vec3(0,1,0)),//NEGATIVE_X - rotate left 90 degrees
	angleAxisMat4(float(M_PI_2), vec3(0,1,0)),//POSITIVE_X - rotate right 90 degrees

	angleAxisMat4(float(-M_PI_2), vec3(1,0,0)) * angleAxisMat4(float(M_PI), vec3(0,1,0)),//POSITIVE_Y - rotate up 90 degrees
	angleAxisMat4(float(M_PI_2), vec3(1,0,0)) * angleAxisMat4(float(M_PI), vec3(0,1,0)),//NEGATIVE_Y - rotate down 90 degrees


	angleAxisMat4(float(M_PI), vec3(0,1,0)),     //POSITIVE_Z - rotate right 180 degrees
	mat4::Identity()                           //NEGATIVE_Z
}};

CubemapRenderer::CubemapRenderer(size_t facePixelWidth, Format format)
	:format_(format),
	drawIfNonZeroProgram_(std::vector<std::string>{
		GLUE_SHADER_DIR + "DrawIfNonZero.vert",
		GLUE_SHADER_DIR + "DrawIfNonZero.frag"
	}),
	drawTexProgram_({
		GLUE_SHADER_DIR + "FullScreenTex_Cubemap.vert",
		GLUE_SHADER_DIR + "FullScreenTex_Cubemap.frag"
	}),
	omniToCubemapProgram_({
		GLUE_SHADER_DIR + "OmniToCubemap.vert",
		GLUE_SHADER_DIR + "OmniToCubemap.frag"
	}),
	facePixelWidth_(facePixelWidth)
{
	std::cout << "** Cubemap Rotations **" << std::endl;
	for (size_t face = 0; face < 6; ++face) {
		std::cout << "Face " << face << std::endl;
		std::cout << cubemapRotations_[face] << std::endl;
		std::cout << "x 0,0,-1" << std::endl;
		std::cout << cubemapRotations_[face].block<3, 3>(0, 0) * vec3(0, 0, -1) << std::endl;
		std::cout << "x 0,1,-1" << std::endl;
		std::cout << cubemapRotations_[face].block<3, 3>(0, 0) * vec3(0, 1, -1) << std::endl;
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glGenFramebuffers(1, &framebuffer_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	glGenRenderbuffers(1, &renderbuffer_);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER, renderbuffer_);

	//cuGraphicsGLRegisterImage(&cudaItmTex_, fromItmTex_.tex(), GL_TEXTURE_2D, cudaGraphicsRegisterFlagsSurfaceLoadStore);

	//Create texture
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 1000);
	if(format_ == Format::RGBA8) {
    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	glGenTextures(1, &depthTexture_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthTexture_);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);

	for(int face = 0; face < 6; ++face) {
		//TODO change this to filling with some test data for debugging.
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
		if(format_ == Format::RGBA8) {
    		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA8,
    			GLsizei(facePixelWidth_), GLsizei(facePixelWidth_), 0,
    			GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		} else /* RG32F */ {
    		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RG32F,
    			GLsizei(facePixelWidth_), GLsizei(facePixelWidth_), 0,
    			GL_RG, GL_FLOAT, nullptr);
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthTexture_);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT,
			GLsizei(facePixelWidth_), GLsizei(facePixelWidth_), 0,
			GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

}

CubemapRenderer::~CubemapRenderer() throw()
{
	glDeleteTextures(1, &texture_);
}

void CubemapRenderer::render(Scene *scene, CameraBlockBuffer *camUniformBlock)
{
	render([scene]() {scene->render(); }, camUniformBlock);
}

void CubemapRenderer::render(std::function<void(void)> func,
	CameraBlockBuffer *camUniformBlock)
{
	glEnable(GL_DEPTH_TEST);
	mat4 origin = modelToWorld().inverse();

	CameraBlock cubemapCamBlock;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	mat4 convertMat;
	convertMat << 1.f, 0.f, 0.f, 0.f,
		0.f, -1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f;
	//= angleAxisMat4(-float(M_PI), vec3(1.f, 0.f, 0.f));;

	for(int face = 0; face < 6; ++face) {
		//Prepare texture for rendering.
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture_, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depthTexture_, 0);

		//DEBUG check status of framebuffer
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("CUBEMAP FRAMEBUFFER NOT COMPLETE");
		}
		glViewport(0, 0, GLsizei(facePixelWidth_), GLsizei(facePixelWidth_));

		//Rotate camera
		mat4 t = cubemapRotations_[face]*origin;
		camUniformBlock->block.worldToClip = cubemapPerspective * t;
		camUniformBlock->block.cameraPos = -t.block<4,1>(0,3);
		camUniformBlock->block.cameraDir = t.inverse() * vec4(0,0,1,0);

		camUniformBlock->bindRange(UniformBlock::CAMERA);
		camUniformBlock->update();

		//Render scene
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_CULL_FACE);
		func();
	}

	#ifdef __APPLE__
	//No proper OGL support on OS X (grumble)
	glBindTexture(GL_TEXTURE_2D, texture_);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	#else
	glGenerateTextureMipmap(texture_);
	#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapRenderer::render(std::function<void(const mat4&)> renderFunc)
{
	glEnable(GL_DEPTH_TEST);
	mat4 origin = modelToWorld().inverse();

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	for (int face = 0; face < 6; ++face) {
		//Prepare texture for rendering.
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture_, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depthTexture_, 0);

		//DEBUG check status of framebuffer
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("CUBEMAP FRAMEBUFFER NOT COMPLETE");
		}
		glViewport(0, 0, GLsizei(facePixelWidth_), GLsizei(facePixelWidth_));

		//Rotate camera
		mat4 t = cubemapRotations_[face] * origin;
		mat4 worldToClip = cubemapPerspective * t;

		//Render scene
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderFunc(worldToClip);
	}

#ifdef __APPLE__
	//No proper OGL support on OS X (grumble)
	glBindTexture(GL_TEXTURE_2D, texture_);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
#else
	glGenerateTextureMipmap(texture_);
#endif

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);;
}

void CubemapRenderer::loadExampleCubemap()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
	cv::Mat m;
	cv::Mat m4chan;

	m = cv::imread(GLUE_MODEL_DIR + "testcubemap/posx.jpg");
	cv::cvtColor(m, m4chan, cv::COLOR_BGR2RGBA);
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0,
		GLsizei(facePixelWidth_), GLsizei(facePixelWidth_),
		GL_RGBA, GL_UNSIGNED_BYTE, m4chan.data);

	m = cv::imread(GLUE_MODEL_DIR + "testcubemap/negx.jpg");
	cv::cvtColor(m, m4chan, cv::COLOR_BGR2RGBA);
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0,
		GLsizei(facePixelWidth_), GLsizei(facePixelWidth_),
		GL_RGBA, GL_UNSIGNED_BYTE, m4chan.data);

	m = cv::imread(GLUE_MODEL_DIR + "testcubemap/posy.jpg");
	cv::cvtColor(m, m4chan, cv::COLOR_BGR2RGBA);
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0,
		GLsizei(facePixelWidth_), GLsizei(facePixelWidth_),
		GL_RGBA, GL_UNSIGNED_BYTE, m4chan.data);

	m = cv::imread(GLUE_MODEL_DIR + "testcubemap/negy.jpg");
	cv::cvtColor(m, m4chan, cv::COLOR_BGR2RGBA);
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0,
		GLsizei(facePixelWidth_), GLsizei(facePixelWidth_),
		GL_RGBA, GL_UNSIGNED_BYTE, m4chan.data);

	m = cv::imread(GLUE_MODEL_DIR + "testcubemap/posz.jpg");
	cv::cvtColor(m, m4chan, cv::COLOR_BGR2RGBA);
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0,
		GLsizei(facePixelWidth_), GLsizei(facePixelWidth_),
		GL_RGBA, GL_UNSIGNED_BYTE, m4chan.data);

	m = cv::imread(GLUE_MODEL_DIR + "testcubemap/negz.jpg");
	cv::cvtColor(m, m4chan, cv::COLOR_BGR2RGBA);
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0,
		GLsizei(facePixelWidth_), GLsizei(facePixelWidth_),
		GL_RGBA, GL_UNSIGNED_BYTE, m4chan.data);


#ifdef __APPLE__
	//No proper OGL support on OS X (grumble)
	glBindTexture(GL_TEXTURE_2D, texture_);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
#else
	glGenerateTextureMipmap(texture_);
#endif

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

void CubemapRenderer::addOmniImageSimple(Texture *omniIm, Texture *omniImMask,
	const mat4 &omniCamPose)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
	glDisable(GL_DEPTH_TEST);

	for(int face = 0; face < 6; ++face) {

		//Prepare texture for rendering.
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture_, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depthTexture_, 0);

		//DEBUG check status of framebuffer
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("CUBEMAP FRAMEBUFFER NOT COMPLETE");
		}
		glViewport(0, 0, GLsizei(facePixelWidth_), GLsizei(facePixelWidth_));

		//Find full transform from current face space to omni cam space.
		mat4 modelToOmni = omniCamPose * modelToWorld();
		//Take the rotational component of the transform.
		mat3 cubeRot = cubemapRotations_[face].inverse().block<3, 3>(0, 0);
		mat3 rot = modelToOmni.block<3, 3>(0, 0) * cubeRot;

		omniToCubemapProgram_.use();
		omniToCubemapProgram_.setUniform("rotFaceToOmni", rot);
		omniIm->bindToImageUnit(1);
		omniToCubemapProgram_.setUniform("omniIm", 1);
		omniImMask->bindToImageUnit(2);
		omniToCubemapProgram_.setUniform("omniImMask", 2);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		FullScreenQuad::getInstance().render(omniToCubemapProgram_);
		omniToCubemapProgram_.unuse();
	}
	omniIm->unbind();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	#ifdef __APPLE__
	//No proper OGL support on OS X (grumble)
	glBindTexture(GL_TEXTURE_2D, texture_);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	#else
	glGenerateTextureMipmap(texture_);
	#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubemapRenderer::getImage(int face, void *buffer)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
	if(format_ == Format::RGBA8) {
    	glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
    		GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	} else /* RG32F */ {
    	glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
    		GL_RG, GL_FLOAT, buffer);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubemapRenderer::update(int face, void *buffer)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
	if(format_ == Format::RGBA8) {
    	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
    		GL_RGBA, GLsizei(facePixelWidth_), GLsizei(facePixelWidth_), 
			0, GL_UNSIGNED_BYTE, GL_RGBA, buffer);
	} else /* RG32F */ {
    	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
    		GL_RG32F, GLsizei(facePixelWidth_), GLsizei(facePixelWidth_), 
			0, GL_FLOAT, GL_RG, buffer);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

GLuint CubemapRenderer::texture()
{
	return texture_;
}

void CubemapRenderer::renderNet(
	Viewport vBig)
{
	Viewport v = fitViewportWithAspectRatio(vBig, 4.f / 3.f);
	int dx = int(floor(v.w / 4));
	int dy = int(floor(v.h / 3));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
	drawTexProgram_.setUniform("tex", 0);

	mat3 rot = cubemapRotations_[0].block<3, 3>(0, 0).inverse();
	drawTexProgram_.setUniform("rot", rot);
	glViewport(v.x + 2*dx, v.y + dy, dx, dy);
	FullScreenQuad::getInstance().render(drawTexProgram_);

	rot = cubemapRotations_[1].block<3, 3>(0, 0).inverse();
	drawTexProgram_.setUniform("rot", rot);
	glViewport(v.x, v.y + dy, dx, dy);
	FullScreenQuad::getInstance().render(drawTexProgram_);

	rot = cubemapRotations_[2].block<3, 3>(0, 0).inverse();
	drawTexProgram_.setUniform("rot", rot);
	glViewport(v.x + dx, v.y + 2*dy, dx, dy);
	FullScreenQuad::getInstance().render(drawTexProgram_);

	rot = cubemapRotations_[3].block<3, 3>(0, 0).inverse();
	drawTexProgram_.setUniform("rot", rot);
	glViewport(v.x + dx, v.y, dx, dy);
	FullScreenQuad::getInstance().render(drawTexProgram_);

	rot = cubemapRotations_[4].block<3, 3>(0, 0).inverse();
	drawTexProgram_.setUniform("rot", rot);
	glViewport(v.x + dx, v.y + dy, dx, dy);
	FullScreenQuad::getInstance().render(drawTexProgram_);

	rot = cubemapRotations_[5].block<3, 3>(0, 0).inverse();
	drawTexProgram_.setUniform("rot", rot);
	glViewport(v.x + 3*dx, v.y + dy, dx, dy);
	FullScreenQuad::getInstance().render(drawTexProgram_);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

const std::array<mat4, 6> &CubemapRenderer::cubemapRotations()
{
	return cubemapRotations_;
}

}
