#include "glue/PrtMeshAlbedoShadowed.hpp"

namespace glue {

PrtMeshAlbedoShadowed::PrtMeshAlbedoShadowed(const std::string &meshFilename,
	const std::string &prtCoefftsFilename,
	const std::string &shadowMeshFilename,
	const std::string &albedoTexFilename)
:PrtMesh(SHADOW_ALBEDO, meshFilename, prtCoefftsFilename, shadowMeshFilename, albedoTexFilename)
{}

PrtMeshAlbedoShadowed::~PrtMeshAlbedoShadowed() throw()
{}

}
