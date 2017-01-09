#pragma once

#include <optixu/optixu_vector_types.h>
#include <optixu/optixu_math_stream_namespace.h>


///
/// \brief The LightShape struct
///
struct LightShape
{

  enum LightShapes
  {

    SPHERE,
//    IRRADIANCE_SPHERE,
    NUM_LIGHT_SHAPES

  };

};



///
/// \brief The Light struct
///
struct Illuminator
{

#if defined( __cplusplus )
  typedef optix::float3 float3;
#endif

  float3 center;                 // 12 : 12
  float3 radiantFlux;            // 12 : 24
  LightShape::LightShapes shape; // 4  : 28
  float radius;                  // 4  : 32

};

