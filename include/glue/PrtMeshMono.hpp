#ifndef GLUE_PRTMESHMONO_HPP_INCLUDED
#define GLUE_PRTMESHMONO_HPP_INCLUDED

#include "glue/PrtMesh.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh.
class PrtMeshMono : public PrtMesh
{
public:
	//!\brief Constructor for Mono PRT mesh.
	PrtMeshMono(const std::string &meshFilename, const std::string &prtCoefftsFilename);

	~PrtMeshMono() throw();
	
private:
	PrtMeshMono(const PrtMeshMono &);
	PrtMeshMono &operator=(const PrtMeshMono &);
};

}

#endif
