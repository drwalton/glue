set(ASSIMP_DIR "C:/Program Files/Assimp/")

find_path(ASSIMP_INCLUDE_DIRS
	NAMES
		assimp/Importer.hpp
	PATHS
		"${ASSIMP_DIR}/include"
		$ENV{WIN_LOCAL_DIR}/assimp/include
		"F:/Program Files/Assimp/include"
)

find_library(ASSIMP_LIBRARIES
	NAMES
		assimp
	PATHS
		"${ASSIMP_DIR}/lib/x64"
		$ENV{WIN_LOCAL_DIR}/assimp/assimp-3.2/lib/
		"F:/Program Files/Assimp/lib/x64"
)

message("Found ASSIMP: LIB: ${ASSIMP_LIBRARIES} INCLUDE: ${ASSIMP_INCLUDE_DIRS}")

