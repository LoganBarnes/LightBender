#include "optix.h"
#include "RendererObjects.hpp"
#include "path_tracer.h"
#include "random.h"



struct PerRayData_pathtrace
{

  float3 result;
  float3 radiance;
  float3 attenuation;
  float3 origin;
  float3 direction;
  unsigned seed;
  int depth;
  int countEmitted;
  int done;
  int inside;

};


rtDeclareVariable( PerRayData_radiance, prd_radiance,      rtPayload, );

rtDeclareVariable( optix::Ray,          ray,               rtCurrentRay, );
rtDeclareVariable( uint2,               launch_index,      rtLaunchIndex, );
rtDeclareVariable( unsigned int,        frame_number,      , );
rtDeclareVariable( unsigned int,        sqrt_num_samples,  , );

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

  float2 inv_screen  = 1.0f / make_float2( screenSize ) * 2.0f;
  float2 pixelCorner = make_float2( launch_index ) * inv_screen - 1.0f;

  float2 jitter_scale            = inv_screen / sqrt_num_samples;
  unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;

  float3 totalRadiance = make_float3( 0.0f );

  // loop vars
  unsigned x, y;
  float2 jitter;

  // faster two for loops for x and y on gpu?
  while ( samples_per_pixel-- )
  {

    x = samples_per_pixel % sqrt_num_samples;
    y = samples_per_pixel / sqrt_num_samples;

    // no random jitter, just center value
    jitter = make_float2( x + 0.5f, y + 0.5f );

    float2 d             = pixelCorner + jitter * jitter_scale;
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

    totalRadiance += prd.result;

  }

  totalRadiance /= sqrt_num_samples * sqrt_num_samples ;

  output_buffer[ launch_index ] = make_float4( totalRadiance, 1.0 );

} // pinhole_camera



/////////////////////////////////////////////////////////
/// \brief pinhole_camera
/////////////////////////////////////////////////////////
RT_PROGRAM
void
pathtrace_pinhole_camera( )
{

//  size_t2 screenSize = output_buffer.size( );

//  float2 screenSizeInv = 1.0f / make_float2( screenSize ) * 2.f;
//  float2 pixel         = ( make_float2( launch_index ) ) * screenSizeInv - 1.f;

//  float2 jitter_scale            = screenSizeInv / sqrt_num_samples;
//  unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;
//  float3 result                  = make_float3( 0.0f );

//  unsigned int seed = tea< 16 >( screenSize.x * launch_index.y + launch_index.x, frame_number );

//  do
//  {

//    unsigned int x       = samples_per_pixel % sqrt_num_samples;
//    unsigned int y       = samples_per_pixel / sqrt_num_samples;
//    float2 jitter        = make_float2( x - rnd( seed ), y - rnd( seed ) );
//    float2 d             = pixel + jitter * jitter_scale;
//    float3 ray_origin    = eye;
//    float3 ray_direction = normalize( d.x * U + d.y * V + W );

//    PerRayData_pathtrace prd;
//    prd.result       = make_float3( 0.f );
//    prd.attenuation  = make_float3( 1.f );
//    prd.countEmitted = true;
//    prd.done         = false;
//    prd.inside       = false;
//    prd.seed         = seed;
//    prd.depth        = 0;

//    for ( ; ; )
//    {

//      Ray ray = make_Ray( ray_origin,
//                         ray_direction,
//                         pathtrace_ray_type,
//                         scene_epsilon,
//                         RT_DEFAULT_MAX );

//      rtTrace( top_object, ray, prd );

//      if ( prd.done )
//      {

//        prd.result += prd.radiance * prd.attenuation;
//        break;

//      }

//      // RR
//      if ( prd.depth >= rr_begin_depth )
//      {

//        float pcont = fmaxf( prd.attenuation );

//        if ( rnd( prd.seed ) >= pcont )
//        {

//          break;

//        }

//        prd.attenuation /= pcont;

//      }

//      prd.depth++;
//      prd.result   += prd.radiance * prd.attenuation;
//      ray_origin    = prd.origin;
//      ray_direction = prd.direction;

//    } // eye ray

//    result += prd.result;
//    seed    = prd.seed;

//  }
//  while ( --samples_per_pixel );

//  float3 pixel_color = result / ( sqrt_num_samples * sqrt_num_samples );

//  if ( frame_number > 1 )
//  {

//    float a          = 1.0f / ( float ) frame_number;
//    float b          = ( ( float ) frame_number - 1.0f ) * a;
//    float3 old_color = make_float3( output_buffer[ launch_index ] );
//    output_buffer[ launch_index ] = make_float4( a * pixel_color + b * old_color, 0.0f );

//  }
//  else
//  {

//    output_buffer[ launch_index ] = make_float4( pixel_color, 0.0f );

//  }

  size_t2 screenSize = output_buffer.size( );

  float2 inv_screen  = 1.0f / make_float2( screenSize ) * 2.0f;
  float2 pixelCorner = make_float2( launch_index ) * inv_screen - 1.0f;

  float2 jitter_scale            = inv_screen / sqrt_num_samples;
  unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;

  float3 totalRadiance = make_float3( 0.0f );

  // loop vars
  unsigned x, y;
  float2 jitter;

  unsigned seed = tea< 16 >( screenSize.x * launch_index.y + launch_index.x, frame_number );

  // faster two for loops for x and y on gpu?
  while ( samples_per_pixel-- )
  {

    x = samples_per_pixel % sqrt_num_samples;
    y = samples_per_pixel / sqrt_num_samples;

    // no random jitter, just center value
    jitter = make_float2( x + rnd( seed ), y + rnd( seed ) );

    float2 d             = pixelCorner + jitter * jitter_scale;
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

    totalRadiance += prd.result;

  }

  totalRadiance /= sqrt_num_samples * sqrt_num_samples ;

  if ( frame_number > 1 )
  {

    float a = 1.0f / static_cast< float >( frame_number );
    float b = ( static_cast< float >( frame_number ) - 1.0f ) * a;
    float3 oldRadiance = make_float3( output_buffer[ launch_index ] );
    output_buffer[ launch_index ] = make_float4( a * totalRadiance + b * oldRadiance, 1.0f );

  }
  else
  {

    output_buffer[ launch_index ] = make_float4( totalRadiance, 1.0f );

  }

} // pinhole_camera



/////////////////////////////////////////////////////////
/// \brief orthographic_camera
/////////////////////////////////////////////////////////
RT_PROGRAM
void
orthographic_camera( )
{

  size_t2 screenSize = output_buffer.size( );

  float2 pixelCorner   = make_float2( launch_index );
  float2 d             = ( pixelCorner + 0.5 ) / make_float2( screenSize ) * 2.f - 1.f; // film coords
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



/////////////////////////////////////////////////////////
/// \brief pathtrace_orthographic_camera
/////////////////////////////////////////////////////////
RT_PROGRAM
void
pathtrace_orthographic_camera( )
{

  size_t2 screenSize = output_buffer.size( );

  float2 pixelCorner   = make_float2( launch_index );
  float2 d             = ( pixelCorner + 0.5 ) / make_float2( screenSize ) * 2.f - 1.f; // film coords
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

  float4 radiance = make_float4( prd.result, 1.0 );

  if ( frame_number > 1 )
  {

    float a = 1.0f / static_cast< float >( frame_number );
    float b = ( static_cast< float >( frame_number ) - 1.0f ) * a;
    output_buffer[ launch_index ] = a * radiance + b * output_buffer[ launch_index ];

  }
  else
  {

    output_buffer[ launch_index ] = radiance;

  }

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
