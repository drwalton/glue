#include "glue/PrtMeshAlbedo.hpp"

namespace glue {

PrtMeshAlbedo::PrtMeshAlbedo(const std::string &meshFilename, 
	const std::string &prtCoefftsFilename,
	const std::string &albedoTexFilename)
	:PrtMesh(ALBEDO, meshFilename, prtCoefftsFilename, "", albedoTexFilename)
{
}

PrtMeshAlbedo::~PrtMeshAlbedo() throw()
{
}


}
