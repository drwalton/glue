#include "glue/PrtMeshMonoShadowedLdr.hpp"

namespace glue {

PrtMeshMonoShadowedLdr::PrtMeshMonoShadowedLdr(
	const std::string & meshFilename, 
	const std::string & prtCoefftsFilename, 
	const std::string &shadowMeshFilename)
	:PrtMesh(SHADOW_MONO_LDR, meshFilename, prtCoefftsFilename, shadowMeshFilename, "")
{
}

PrtMeshMonoShadowedLdr::~PrtMeshMonoShadowedLdr() throw()
{
}

}
