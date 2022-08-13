find_package(Assimp)
find_package(Boost COMPONENTS Filesystem System REQUIRED)
find_package(OpenGL REQUIRED)

function(add_glue_test TEST_NAME)
	if(WIN32)
		add_executable(${TEST_NAME} "${TEST_NAME}.cpp")
	else()
		add_executable(${TEST_NAME} "${TEST_NAME}.cpp")
	endif()
	target_link_libraries(${TEST_NAME}
		glue
		AntTweakBar
		${SDL_LIBRARIES}
		${ASSIMP_LIBRARIES}
		${FLTK_LIBRARIES}
		${SDL_ttf_LIBRARIES}
		${GLEW_LIBRARIES}
		${OPENGL_LIBRARIES}
		${Boost_LIBRARIES}
		${OPENCV_LIBRARIES}
	)
	set_target_properties(${TEST_NAME} PROPERTIES FOLDER Test)
endfunction()

add_glue_test(TestAntTweakBar)
add_glue_test(TestBakeAmbientOcclusion)
add_glue_test(TestComputeShaderImage)
add_glue_test(TestGLWindow)
add_glue_test(TestFastDirSh)
add_glue_test(TestFullScreenQuad)
add_glue_test(TestFullScreenQuadFractal)
add_glue_test(TestArrayTexture)
add_glue_test(TestPrtArrayTexture)
add_glue_test(TestBakeDiffuseShadowedPRT)
add_glue_test(TestDirToSphericalAngles)
add_glue_test(TestProjectCubemapSH)
add_glue_test(TestSampleCubemap)
add_glue_test(TestMakeSHCubemap)
add_glue_test(TestRenderMeshShadowPrt)
add_glue_test(TestRenderMeshPrt)
add_glue_test(TestRenderMeshColorShadowPrt)
add_glue_test(TestRenderToTexture)
add_glue_test(TestTexturedMeshPhongShadow)
add_glue_test(TestRotateViewer)
add_glue_test(TestFlyViewer)
add_glue_test(TestTransformFeedback)
add_glue_test(TestTransformFeedbackGeomShader)
add_glue_test(TestWaveMesh)

add_glue_test(TestMessageBox)

add_glue_test(TestMakeUniqueSubdirectory)
add_glue_test(TestSurfelCloud)

message("libs:${SDL_LIBRARIES}
	${SDL_ttf_LIBRARIES}
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
	${Boost_LIBRARIES}")
