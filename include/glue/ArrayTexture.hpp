#ifndef GLUE_ARRAYTEXTURE_HPP_INCLUDED
#define GLUE_ARRAYTEXTURE_HPP_INCLUDED

#include <GL/glew.h>
#include <string>

namespace glue {

class ArrayTexture final
{
public:
	explicit ArrayTexture(
		size_t width, size_t height, size_t nLayers, 
		GLenum format, GLenum type,
		GLenum internalFormat);
	//!\brief Sets the format in which PRT textures are saved on the GPU.
	enum class PrtLoadMode {
		UCHAR, //One byte per channel, per pixel. 
		FLOAT,  //One float per channel, per pixel.	
		FLOAT_COLOR //3 floats per channel, per pixel.
	};
	explicit ArrayTexture(
		const std::string prtTextureFile,
		PrtLoadMode mode = PrtLoadMode::FLOAT);
	~ArrayTexture() throw();

	void update(size_t layer, const void *data);
	template<typename ArrayType>
	void update(const ArrayType &arr) {
		for (size_t i = 0; i < arr.size(); ++i) {
			update(i, reinterpret_cast<const void*>(arr[i]));
		}
	}

	GLuint tex();

	void getData(size_t layer, void *data, size_t buffSize);

	size_t width() const;
	size_t height() const;
	size_t nLayers() const;

	void bindToImageUnit(GLuint unit);

private:
	ArrayTexture(const ArrayTexture&);
	ArrayTexture &operator=(const ArrayTexture&);

	void init(
		size_t width, size_t height, size_t nLayers,
		GLenum format, GLenum type,
		GLenum internalFormat);

	GLuint texture_;

	size_t width_, height_, nLayers_;
	GLenum format_, type_, internalFormat_;
};

}

#endif
