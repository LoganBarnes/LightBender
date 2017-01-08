cmake_minimum_required ( VERSION 3.2.0 )
project ( LightBender )


option( BUILD_TESTS OFF "Build unit tests created with gmock/gtest framework" )

if ( MSVC )
  add_definitions( -DNOMINMAX ) # for OptiX
endif( )

# namespace used for project
set ( PROJECT_NAMESPACE light )

# The version number.
set ( PROJECT_VERSION_MAJOR 1 )
set ( PROJECT_VERSION_MINOR 0 )
set ( PROJECT_VERSION_PATCH 0 )

# required before DefaultProjectLibrary.cmake
set( SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src )

# other paths
set( SHARED_PATH ${PROJECT_SOURCE_DIR}/shared )
set( RES_PATH    ${PROJECT_SOURCE_DIR}/res    )
set( SHADER_PATH ${SHARED_PATH}/shaders       )

set( MODEL_PATH  ${PROJECT_SOURCE_DIR}/run/models )
set( INPUT_PATH  ${PROJECT_SOURCE_DIR}/run/input  )
set( OUTPUT_PATH ${PROJECT_SOURCE_DIR}/run/output )

set( PROJECT_CONFIG_FILE ${SHARED_PATH}/src/common/ProjectConfig.hpp.in )


set( USE_GLFW  ON CACHE BOOL "" FORCE )
set( USE_GLM   ON CACHE BOOL "" FORCE )
set( USE_OPTIX ON CACHE BOOL "" FORCE )
set( USE_GUI   ON CACHE BOOL "" FORCE )

set( USE_GMOCK ${BUILD_TESTS} CACHE BOOL "" FORCE )


# set variables from shared simulation project
add_subdirectory( ${SHARED_PATH} )

# system header dirs
set(
    PROJECT_SYSTEM_INCLUDE_DIRS
    ${SHARED_SYSTEM_INCLUDE_DIRS}
    )

# libraries to link against
set(
    PROJECT_LINK_LIBS
    ${SHARED_LINK_LIBS}
    )

# must be built before project lib
set(
    PROJECT_DEP_TARGETS
    ${SHARED_DEP_TARGETS}
    )



set(
    PROJECT_CUDA_SOURCE

    ${SHARED_PATH}/src/cuda/CudaWrappers.cuh
    ${SHARED_PATH}/src/cuda/CudaWrappers.cu
    ${SHARED_PATH}/src/cuda/CudaRandom.cuh
    ${SHARED_PATH}/src/cuda/CudaRandom.cu

    )

set( PTX_SOURCE

    ${SRC_DIR}/renderers/gpu/cuda/primitives/Parallelogram.cu
    ${SRC_DIR}/renderers/gpu/cuda/primitives/Box.cu
    ${SRC_DIR}/renderers/gpu/cuda/primitives/Sphere.cu
    ${SRC_DIR}/renderers/gpu/cuda/primitives/TriangleMesh.cu

    ${SRC_DIR}/renderers/gpu/cuda/Cameras.cu
    ${SRC_DIR}/renderers/gpu/cuda/Brdf.cu
    )

set(
    PROJECT_CUDA_INCLUDE_DIRS

    ${SHARED_INCLUDE_DIRS}
    ${SHARED_CUDA_INCLUDE_DIRS}

    ${SHARED_PATH}/src/cuda
    ${SRC_DIR}/renderers
    )

set(
    PROJECT_CUDA_SYSTEM_INCLUDE_DIRS

    ${SHARED_SYSTEM_INCLUDE_DIRS}
    ${SHARED_CUDA_SYSTEM_INCLUDE_DIRS}

    ${SHARED_PATH}/src/cuda/helpers
    )

set(
    PROJECT_CUDA_LINK_LIBS
    ${SHARED_CUDA_LINK_LIBS}
    )

set(
    PROJECT_CUDA_DEP_TARGETS
    ${SHARED_CUDA_DEP_TARGETS}
    )


set( USE_CUDA ON )


# header dirs
set(
    PROJECT_INCLUDE_DIRS

    ${SHARED_INCLUDE_DIRS}

    ${SRC_DIR}/io
    ${SRC_DIR}/renderers
    ${SRC_DIR}/renderers/gpu
    ${SRC_DIR}/renderers/gpu/cuda
    )

# cpp files
set(
    PROJECT_SOURCE

    # ignored by compilers but added to project files
    ${SHARED_UNCRUSTIFY_FILE}
    ${SHADER_PATH}/screenSpace/shader.vert
    ${SHADER_PATH}/blending/shader.frag

    ${SHARED_SOURCE}
    ${PTX_SOURCE}

    ${SRC_DIR}/renderers/gpu/OptixRenderer.cpp
    ${SRC_DIR}/renderers/gpu/OptixScene.cpp
    ${SRC_DIR}/renderers/gpu/OptixBasicScene.cpp
    ${SRC_DIR}/renderers/gpu/OptixAdvancedScene.cpp
    ${SRC_DIR}/renderers/gpu/OptixFileScene.cpp
    ${SRC_DIR}/renderers/gpu/OptixModelScene.cpp

    ${SRC_DIR}/io/LightBenderIOHandler.cpp
    ${SRC_DIR}/io/LightBenderCallback.cpp
    )


set(
    TESTING_SOURCE
    ${SRC_DIR}/testing/PathMathUnitTests.cpp
    )

set(
    TESTING_LINK_LIBS
    ${GMOCK_BOTH_LIBS}
    ${SHARED_LINK_LIBS}
    )

set(
    TESTING_DEP_TARGETS
    ${GMOCK_BOTH_LIBS}
    )

set(
    TESTING_SYSTEM_INCLUDE_DIRS
    ${GMOCK_INCLUDE_DIRS}
    ${SRC_DIR}/renderers/gpu/cuda
    )

set(
    TESTING_INCLUDE_DIRS
    ${SRC_DIR}/testing
    )


# file with main function
set( PROJECT_MAIN ${SRC_DIR}/exec/LightBender.cpp )

include( ${SHARED_PATH}/cmake/DefaultProjectLibrary.cmake )

