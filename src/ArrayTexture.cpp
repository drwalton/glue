#include "glue/ArrayTexture.hpp"
#include "glue/Bake.hpp"
#include "glue/Exception.hpp"

void glue::ArrayTexture::init(size_t width, size_t height, size_t nLayers, GLenum format, GLenum type, GLenum internalFormat)
{
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat_, 
		GLsizei(width), GLsizei(height), GLsizei(nLayers), 
		0, format_, type_, nullptr);
	glGetError();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	GLint memory;
	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &memory);
	std::cout << "After allocation, " << memory << "KB of GPU memory remain." << std::endl;
}

glue::ArrayTexture::ArrayTexture(
	size_t width, size_t height, size_t nLayers,
	GLenum format, GLenum type,
	GLenum internalFormat)
	:width_(width), height_(height), nLayers_(nLayers),
	format_(format), type_(type),
	internalFormat_(internalFormat)
{
	init(width, height, nLayers, format, type, internalFormat);
}

glue::ArrayTexture::ArrayTexture(const std::string prtTextureFile, PrtLoadMode mode)
{
	if (mode == PrtLoadMode::FLOAT_COLOR) {
		PrtCoeffts<vec3> prtTextures = loadUncompressedPrtCoeffts<vec3>(prtTextureFile);
		nLayers_ = prtTextures.size();
		format_ = GL_RGB;
		type_ = GL_FLOAT;
		internalFormat_ = GL_RGB32F;
		width_ = prtTextures.width(); height_ = prtTextures.height();
		init(width_, height_, prtTextures.size(), format_, type_, internalFormat_);
		glGetError();
		update(prtTextures);
	} else {
		PrtCoeffts<float> prtTextures = loadUncompressedPrtCoeffts<float>(prtTextureFile);
		nLayers_ = prtTextures.size();
		format_ = GL_RED;
		type_ = GL_FLOAT;
		if (mode == PrtLoadMode::FLOAT) {
			internalFormat_ = GL_R32F;
		} else /* mode == PrtLoadMode::UCHAR */ {
			//TODO also rescale appropriately.
			internalFormat_ = GL_R8;
		}
		width_ = prtTextures.width(); height_ = prtTextures.height();
		init(width_, height_, prtTextures.size(), format_, type_, internalFormat_);
		glGetError();
		update(prtTextures);
	}
}

glue::ArrayTexture::~ArrayTexture() throw()
{
	glDeleteTextures(1, &texture_);
}

void glue::ArrayTexture::update(size_t layer, const void * data)
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 
		GLsizei(layer), GLsizei(width_), GLsizei(height_), 
		1, format_, type_, data);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

GLuint glue::ArrayTexture::tex()
{
	return texture_;
}

void glue::ArrayTexture::getData(
	size_t layer, void * data, size_t buffSize)
{
#ifdef glGetTextureSubImage
	glGetTextureSubImage(texture_, 0, 0, 0, 
		GLsizei(layer), GLsizei(width_), GLsizei(height_), 1, 
		format_, type_, GLsizei(buffSize), data);
#else 
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, 
		GL_COLOR_ATTACHMENT0, texture_, 0, layer);
	glReadPixels(0, 0, width_, height_, format_, type_, data);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
#endif
}

size_t glue::ArrayTexture::width() const
{
	return width_;
}

size_t glue::ArrayTexture::height() const
{
	return height_;
}

size_t glue::ArrayTexture::nLayers() const
{
	return nLayers_;
}

void glue::ArrayTexture::bindToImageUnit(GLuint unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_);
}

