cmake_minimum_required(VERSION 3.6)

project(glue LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
#Disables Boost's auto-linking, which often causes problems for me on Windows.
add_definitions( -DBOOST_ALL_NO_LIB )
#Needs to be defined for some libraries to link correctly (e.g.
# program_options).
add_definitions( -DBOOST_ALL_DYN_LINK  )
endif(WIN32)

option(COMPILE_TESTS "Compile a series of test programs" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

if(APPLE)
	set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif(APPLE)
if(UNIX)
	set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -pthread -fPIC")
endif(UNIX)

find_package(Eigen REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(Boost COMPONENTS System Filesystem REQUIRED)
find_package(Assimp REQUIRED)
find_package(nanort REQUIRED)
find_package(OpenCV REQUIRED)
find_package(FLTK REQUIRED)
find_package(RenderDoc)
set(GLEW_LIBRARIES ${GLEW_LIBRARY})

add_definitions(-DTW_STATIC)
include_directories(3rdParty/AntTweakBar_sdl2/include)
include_directories(${EIGEN_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${SDL_INCLUDE_DIRS})
include_directories(${SDL_ttf_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ASSIMP_INCLUDE_DIRS})
include_directories(${NANORT_INCLUDE_DIRS})
include_directories(${OPENCV_INCLUDE_DIRS})
include_directories(${FLTK_INCLUDE_DIR})
if(RenderDoc_INCLUDE_DIRS)
	include_directories(${RenderDoc_INCLUDE_DIRS})
endif()

#link_directories(${Boost_LIBRARY_DIR_DEBUG} ${Boost_LIBRARY_DIR_RELEASE})

#Autogenerate directories file.
file(WRITE include/glue/Directories.hpp "\
\#ifndef GLUE_DIRECTORIES_HPP_INCLUDED\n\
\#define GLUE_DIRECTORIES_HPP_INCLUDED\n\
\n
#include <string>\n
namespace glue {\n\
	//!Directory containing GLUE's built-in shader source.\n
	const std::string GLUE_SHADER_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/shader/\";\n\
	const std::string GLUE_FONT_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/font/\";\n\
	const std::string GLUE_MODEL_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/model/\";\n\
	const std::string GLUE_CONFIG_DIR = \"${CMAKE_CURRENT_SOURCE_DIR}/config/\";\n\
}\n\
\n
\#endif\n\
")

include_directories(include)
add_subdirectory(3rdParty)
add_subdirectory(src)
add_subdirectory(tools)

if(COMPILE_TESTS)
add_subdirectory(test)
endif(COMPILE_TESTS)
