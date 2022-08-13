#include "glue/PrtMeshColorShadowed.hpp"

namespace glue {

PrtMeshColorShadowed::PrtMeshColorShadowed(const std::string &meshFilename, 
	const std::string &prtCoefftsFilename,
	const std::string &shadowMeshFilename)
	:PrtMesh(SHADOW_COLOR, meshFilename, prtCoefftsFilename, shadowMeshFilename, "")
{
}

PrtMeshColorShadowed::~PrtMeshColorShadowed() throw()
{
}


}
