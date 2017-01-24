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



struct Material
{

#if defined( __cplusplus )
  typedef optix::float3 float3;
#endif

  float3 albedo;    // 12 : 12
  float3 IOR;       // 12 : 24
  float  roughness; // 4  : 28
  float  padding;   // 4  : 32

};




///
/// \brief The SurfaceElement struct
///
struct SurfaceElement
{

#if defined( __cplusplus )
  typedef optix::float3 float3;
  typedef optix::float2 float2;
#endif

  Material material; // 32 : 32
  float3   point;    // 12 : 44
  float3   normal;   // 12 : 56
  float2   padding;  // 8  : 64

};

