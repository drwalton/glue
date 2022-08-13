#ifndef GLUE_PRTMESHALBEDO_HPP_INCLUDED
#define GLUE_PRTMESHALBEDO_HPP_INCLUDED

#include "glue/PrtMesh.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh.
class PrtMeshAlbedo : public PrtMesh
{
public:
	//!\brief Constructor for Mono PRT mesh.
	PrtMeshAlbedo(const std::string &meshFilename,
		const std::string &prtCoefftsFilename,
		const std::string &albedoTexFilename);

	~PrtMeshAlbedo() throw();
	
private:
	PrtMeshAlbedo(const PrtMeshAlbedo &);
	PrtMeshAlbedo &operator=(const PrtMeshAlbedo &);
};

}

#endif
