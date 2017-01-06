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
struct Light
{

#if defined( __cplusplus )
  typedef optix::float3 float3;
#endif

  float3 center;                 // 12 : 12
  float3 radiantFlux;               // 12 : 24
  LightShape::LightShapes shape; // 4  : 28
  float radius;                  // 4  : 32

};



#if defined( __cplusplus )

typedef optix::float3 float3;


///
/// \brief createLight
///
static
Light
createLight(
            float3                  center,
            float3                  powerWatts,
            LightShape::LightShapes shape,
            float                   radius // should be dimensions eventually
            )
{

  Light light;

  light.center      = center;
  light.shape       = shape;
  light.radius      = radius;
  light.radiantFlux = powerWatts;

//  float area;

//  switch ( light.shape )
//  {

//  case LightShape::SPHERE:

//    // sphere area = 4 * pi * r^2
//    area = 4.0f * M_PIf * light.radius * light.radius;

//    break;


//  default:

//    throw std::runtime_error( "Unrecognized light shape" );

//  } // switch

//  light.radiance = powerWatts / ( area * M_PIf ); // W / m^2 sr

  return light;

} // createLight



#endif // if defined( __cplusplus )
