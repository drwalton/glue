#include "glue/PrtMeshMono.hpp"

namespace glue {

PrtMeshMono::PrtMeshMono(const std::string & meshFilename, const std::string & prtCoefftsFilename)
	:PrtMesh(MONO, meshFilename, prtCoefftsFilename, "", "")
{
}

PrtMeshMono::~PrtMeshMono() throw()
{
}

}
