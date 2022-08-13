# glue

Small, cross-platform C++ helper library for OpenGL.

In addition to classes designed for developing 3D rendered applications in OpenGL, glue contains tools for baking and later rendering [Precomputed Radiance Transfer](https://www.ppsloan.org/publications/shillum_final23.pdf) textures for 3D objects. 

## Dependencies

CMake is used as the build system, and generates header files which are required for glue to function.

Requirements:
* Eigen (For matrix datatypes, matrix manipulation)
* GLEW (For managing OpenGL extensions)
* SDL 2 (For window creation, input handling)
* SDL_ttf (For text rendering)
* Boost (Filesystem component is used to handle shader loading, property_tree is used for reading config files)
* FLTK (simple UI)
