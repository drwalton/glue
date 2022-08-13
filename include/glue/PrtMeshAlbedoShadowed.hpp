#ifndef GLUE_PRTMESHALBEDOSHADOWED_HPP_INCLUDED
#define GLUE_PRTMESHALBEDOSHADOWED_HPP_INCLUDED

#include "glue/PrtMesh.hpp"

namespace glue {

	//!\brief Class abstracting a renderable 3D triangle mesh.
	class PrtMeshAlbedoShadowed : public PrtMesh
	{
	public:
		//!\brief Constructor for Mono PRT mesh.
		PrtMeshAlbedoShadowed(const std::string &meshFilename,
			const std::string &prtCoefftsFilename,
			const std::string &shadowMeshFilename,
			const std::string &albedoTexFilename);

		~PrtMeshAlbedoShadowed() throw();

	private:
		PrtMeshAlbedoShadowed(const PrtMeshAlbedoShadowed &);
		PrtMeshAlbedoShadowed &operator=(const PrtMeshAlbedoShadowed &);
	};

}

#endif
