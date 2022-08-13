#ifndef GLUE_PRTCOEFFTS_HPP_INCLUDED
#define GLUE_PRTCOEFFTS_HPP_INCLUDED

#include "glue/SphericalHarmonics.hpp"

namespace glue {

template<typename T>
class PrtCoeffts {
public:
	PrtCoeffts(size_t width, size_t height, size_t nBands)
		:width_(width), height_(height), nBands_(nBands)
	{
		coeffts_.resize(calcNumSHCoeffts(nBands_));
		for(size_t i = 0; i < calcNumSHCoeffts(nBands); ++i) {
			coeffts_[i] = new T[width_*height_];
		}
	}
	PrtCoeffts(const PrtCoeffts& other)
	:width_(other.width()), height_(other.height()), nBands_(other.nBands())
	{
		coeffts_.resize(other.coeffts_.size());
		for(size_t i = 0; i < other.size(); ++i) {
			coeffts_[i] = new T[width_*height_];
			memcpy(coeffts_[i], other.coeffts_[i], width_*height_*sizeof(T));
		}
	}
	PrtCoeffts(PrtCoeffts&& rval)
		:width_(rval.width()), height_(rval.height()), nBands_(rval.nBands())
	{
		coeffts_.resize(rval.coeffts_.size());
		for(size_t i = 0; i < rval.size(); ++i) {
			coeffts_[i] = rval.coeffts_[i];
			rval.coeffts_[i] = nullptr;
		}
	}
	~PrtCoeffts() throw() {
		for(size_t i = 0; i < coeffts_.size(); ++i) {
			if(coeffts_[i] != nullptr) {
				delete[] coeffts_[i];
			}
		}
	}
	T* operator[](size_t i) {
		return coeffts_[i];
	}
	const T* operator[](size_t i) const {
		return coeffts_[i];
	}
	const std::vector<T*> &coeffts() const {
		return coeffts_;
	}
	size_t size() const {
		return coeffts_.size();
	}
	size_t width() const {
		return width_;
	}
	size_t height() const {
		return height_;
	}
	size_t nBands() const {
		return nBands_;
	}
	
private:
	PrtCoeffts &operator=(const PrtCoeffts&);
	std::vector<T*> coeffts_;
	const size_t width_, height_, nBands_;
};

enum class PrtCoefftsType {
	FLOAT, VEC3
};

void saveUncompressedPrtCoeffts(
	const std::string &filename,
	const PrtCoeffts<float>& prtTextures);

void saveUncompressedPrtCoeffts(
	const std::string &filename,
	const PrtCoeffts<vec3> &prtTextures);
	
template<typename T>
PrtCoeffts<T> loadUncompressedPrtCoeffts(const std::string &filename);
template<> PrtCoeffts<float> loadUncompressedPrtCoeffts<float>(const std::string &filename);
template<> PrtCoeffts<vec3> loadUncompressedPrtCoeffts<vec3>(const std::string &filename);

PrtCoefftsType getPrtCoefftType(const std::string &prtFilename);

//!\brief View PRT textures, using OpenCV. They will be converted to BGR888 for viewing.
//!\note In the output, green is positive, blue is negative and black is zero.
//!      Fully saturated colours indicate the maximum coefft value in the whole set.
void visualisePrtCoeffts(const PrtCoeffts<float> &prtCoeffts);

void visualisePrtCoeffts(const PrtCoeffts<vec3> &prtCoeffts);

}

#endif
