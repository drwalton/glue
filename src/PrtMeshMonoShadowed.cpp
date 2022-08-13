#include "glue/PrtMeshMonoShadowed.hpp"

namespace glue {

PrtMeshMonoShadowed::PrtMeshMonoShadowed(
	const std::string & meshFilename, 
	const std::string & prtCoefftsFilename, 
	const std::string &shadowMeshFilename)
	:PrtMesh(SHADOW_MONO, meshFilename, prtCoefftsFilename, shadowMeshFilename, "")
{
}

PrtMeshMonoShadowed::~PrtMeshMonoShadowed() throw()
{
}

}
