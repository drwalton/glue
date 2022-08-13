#ifndef GLUE_PRTMESHMONOSHADOWED_HPP_INCLUDED
#define GLUE_PRTMESHMONOSHADOWED_HPP_INCLUDED

#include "glue/PrtMesh.hpp"

namespace glue {

//!\brief Class abstracting a renderable 3D triangle mesh.
class PrtMeshMonoShadowedLdr : public PrtMesh
{
public:
	//!\brief Constructor for Mono PRT mesh.
	PrtMeshMonoShadowedLdr(
		const std::string & meshFilename, 
		const std::string & prtCoefftsFilename,
		const std::string &shadowMeshFilename);

	~PrtMeshMonoShadowedLdr() throw();
	
private:
	PrtMeshMonoShadowedLdr(const PrtMeshMonoShadowedLdr &);
	PrtMeshMonoShadowedLdr &operator=(const PrtMeshMonoShadowedLdr &);
};

}

#endif
