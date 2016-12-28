#include "optix.h"
#include "RendererObjects.hpp"

rtDeclareVariable( PerRayData_radiance, prd_radiance,      rtPayload, );

rtDeclareVariable( optix::Ray,          ray,               rtCurrentRay, );
rtDeclareVariable( uint2,               launch_index,      rtLaunchIndex, );

rtDeclareVariable( unsigned int,        radiance_ray_type, , );
rtDeclareVariable( float,               scene_epsilon,     , );
rtDeclareVariable( rtObject,            top_object,        , );


//
// Pinhole camera implementation
//
rtDeclareVariable( float3, eye, , );
rtDeclareVariable( float3, U,   , );
rtDeclareVariable( float3, V,   , );
rtDeclareVariable( float3, W,   , );

rtBuffer< float4, 2 >        output_buffer;


/////////////////////////////////////////////////////////
/// \brief pinhole_camera
/////////////////////////////////////////////////////////
RT_PROGRAM
void
pinhole_camera( )
{

  size_t2 screenSize = output_buffer.size( );

  float2 d             = make_float2( launch_index ) / make_float2( screenSize ) * 2.f - 1.f;
  float3 ray_origin    = eye;
  float3 ray_direction = normalize( d.x * U + d.y * V + W );

  optix::Ray ray(
                 ray_origin,
                 ray_direction,
                 radiance_ray_type,
                 scene_epsilon
                 );

  PerRayData_radiance prd;
  prd.importance = 1.0f;
  prd.depth      = 0;

  rtTrace( top_object, ray, prd );

  output_buffer[ launch_index ] = make_float4( prd.result, 1.0 );

} // pinhole_camera



/////////////////////////////////////////////////////////
/// \brief orthographic_camera
/////////////////////////////////////////////////////////
RT_PROGRAM
void
orthographic_camera( )
{

  size_t2 screen = output_buffer.size( );

  float2 d             = make_float2( launch_index ) / make_float2( screen ) * 2.f - 1.f; // film coords
  float3 ray_origin    = eye + d.x * U + d.y * V;                          // eye + offset in film space
  float3 ray_direction = normalize( W );                                   // always parallel view direction

  optix::Ray ray = optix::make_Ray(
                                   ray_origin,
                                   ray_direction,
                                   radiance_ray_type,
                                   scene_epsilon,
                                   RT_DEFAULT_MAX
                                   );

  PerRayData_radiance prd;
  prd.importance = 1.f;
  prd.depth      = 0;

  rtTrace( top_object, ray, prd );

  output_buffer[ launch_index ] = make_float4( prd.result, 1.0 );

} // orthographic_camera



rtDeclareVariable( float3, bg_color, , );

/////////////////////////////////////////////////////////
/// \brief miss
///
///        Set pixel to solid background color when
///        no itersections are detected
/////////////////////////////////////////////////////////
RT_PROGRAM
void
miss( )
{

  prd_radiance.result = bg_color;

}



rtDeclareVariable( float3, error_color, , );

/////////////////////////////////////////////////////////
/// \brief exception
///
///        Set pixel to solid color upon failure
/////////////////////////////////////////////////////////
RT_PROGRAM
void
exception( )
{

  output_buffer[ launch_index ] = make_float4( error_color, 1.0 );

}
