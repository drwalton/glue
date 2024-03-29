#include "glue/PinholeCameraModel.hpp"
#include <Eigen/Dense>

namespace glue {

PinholeCameraModel::PinholeCameraModel(float fx, float fy, float cx, float cy, size_t w, size_t h)
	:CameraModel(fx, fy, cx, cy, w, h),
	K((mat3() << fx, 0.f, cx, 0.f, fy, cy, 0.f, 0.f, 1.f).finished()),
	Kinv(K.inverse())
{}

PinholeCameraModel::~PinholeCameraModel()
{}

vec2 PinholeCameraModel::camToPixel(const vec3 &p) const
{
	vec3 hom = K * p;
	return vec2(hom.x() / hom.z(), hom.y() / hom.z());
}

vec3 PinholeCameraModel::pixelToCam(const vec2 &p, float d) const
{
	return  Kinv * vec3(p.x(), p.y(), d);
}

std::vector<std::unique_ptr<CameraModel> >
	PinholeCameraModel::createPyramidCameraModels(int nLevels) const
{
	std::vector<std::unique_ptr<CameraModel> > models;
	PinholeCameraModel *newModel = new PinholeCameraModel(*this);
	models.reserve(nLevels);
	models.emplace_back(newModel);
	for (size_t level = 1; level < static_cast<size_t>(nLevels); ++level) {
		if (models.back()->w % 2 != 0 || models.back()->h % 2 != 0) {
			throw std::runtime_error("Could not make a " 
				+ std::to_string(nLevels) + "-level pyramid - not divisible!");
		}
		
		int newW = int(models.back()->w / 2);
		int newH = int(models.back()->h / 2);
		float newFx = models.back()->fx * 0.5f;
		float newFy = models.back()->fy * 0.5f;
		float newCx = (this->cx + 0.5f) / static_cast<float>(1 << level) - 0.5f;
		float newCy = (this->cy + 0.5f) / static_cast<float>(1 << level) - 0.5f;

		newModel = new PinholeCameraModel(newFx, newFy, newCx, newCy, newW, newH);
		models.emplace_back(newModel);
	}
	return models;
}

CameraModelType PinholeCameraModel::getType() const
{
	return CameraModelType::PINHOLE;
}

std::unique_ptr<CameraModel> PinholeCameraModel::clone() const
{
	return std::unique_ptr<CameraModel>(new PinholeCameraModel(*this));
}


vec2 PinholeCameraModel::getFovAngles() const
{
	return vec2(
		2.f * atanf((float)((w / fx) / 2.0f)),
		2.f * atanf((float)((h / fy) / 2.0f))
	);
}

}
