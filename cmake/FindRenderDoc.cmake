set(RenderDoc_DIR "C:/Program Files/RenderDoc/")

find_path(RenderDoc_INCLUDE_DIRS
	NAMES
		renderdoc_app.h
	PATHS
		${RenderDoc_DIR}
)

