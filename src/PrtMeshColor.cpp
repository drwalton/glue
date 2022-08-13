#include "glue/PrtMeshColor.hpp"

namespace glue {

PrtMeshColor::PrtMeshColor(const std::string &meshFilename, 
	const std::string &prtCoefftsFilename)
	:PrtMesh(COLOR, meshFilename, prtCoefftsFilename, "", "")
{
}

PrtMeshColor::~PrtMeshColor() throw()
{
}


}
