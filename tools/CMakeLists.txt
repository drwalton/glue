function(BUILD_TOOL TOOL_NAME)
	add_executable(${TOOL_NAME} "${TOOL_NAME}.cpp")
	target_link_libraries(
		${TOOL_NAME}
		glue
		${SDL_LIBRARIES}
		${SDL_ttf_LIBRARIES}
		${GLEW_LIBRARIES}
		${OPENGL_LIBRARIES}
		${Boost_LIBRARIES}
		${ASSIMP_LIBRARIES}
		${FLTK_LIBRARIES}
		${OPENCV_LIBRARIES}
	)
	set_target_properties(${TOOL_NAME} PROPERTIES FOLDER Tools)
endfunction()

BUILD_TOOL(ViewUncompressedPRTTexture)
BUILD_TOOL(ViewUncompressedPRTTextureColor)
BUILD_TOOL(ViewPRTTextureCubemap)
BUILD_TOOL(ViewPRTTextureSHBand)
BUILD_TOOL(ViewPRTTextureAlbedoCubemap)
BUILD_TOOL(ViewPRTTextureShadowCubemap)
BUILD_TOOL(ViewPRTTextureShadowColorCubemap)
BUILD_TOOL(BakeDiffuseShadowedPRTTextures)
BUILD_TOOL(BakeInterreflectedPRTAlbedo)
BUILD_TOOL(MakeDifferentialPRTTextures)
BUILD_TOOL(MakeDifferentialPRTMeshes)
BUILD_TOOL(MakeDifferentialColorPRTTextures)
BUILD_TOOL(ViewSH)
