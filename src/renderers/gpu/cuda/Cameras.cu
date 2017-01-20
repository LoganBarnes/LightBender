#include "optix.h"
#include "RendererObjects.hpp"
#include "path_tracer.h"
#include "random.h"



rtDeclareVariable( PerRayData_pathtrace, prd_current,       rtPayload, );

rtDeclareVariable( optix::Ray,           ray,               rtCurrentRay,  );
rtDeclareVariable( uint2,                launch_index,      rtLaunchIndex, );

rtDeclareVariable( unsigned int,         frame_number,      , );
rtDeclareVariable( unsigned int,         sqrt_num_samples,  , );

rtDeclareVariable( unsigned int,         radiance_ray_type, , );
rtDeclareVariable( float,                scene_epsilon,     , );
rtDeclareVariable( rtObject,             top_object,        , );

rtDeclareVariable( unsigned int,         max_bounces,       , );
rtDeclareVariable( unsigned int,         first_bounce,      , );
rtDeclareVariable( unsigned int,         globalSeed,        , );


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

  float2 jitter_scale = inv_screen / sqrt_num_samples;

  float3 totalRadiance = make_float3( 0.0f );

  // loop vars
  unsigned x, y;
  float2 jitter;

  unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;

  // seems faster than two for loops for x and y on gpu
  while ( samples_per_pixel-- )
  {

    x = samples_per_pixel % sqrt_num_samples;
    y = samples_per_pixel / sqrt_num_samples;

    // no random jitter, just center value
    jitter = make_float2( x + 0.5f, y + 0.5f );

    float2 d             = pixelCorner + jitter * jitter_scale;
    float3 ray_origin    = eye;
    float3 ray_direction = normalize( d.x * U + d.y * V + W );

    PerRayData_pathtrace prd;
    prd.result       = make_float3( 0.f );
    prd.attenuation  = make_float3( 1.f );
    prd.radiance     = make_float3( 0.f );
    prd.countEmitted = true;
    prd.done         = false;
    prd.inside       = false;
    prd.seed         = static_cast< unsigned >( -1 ); // overflow to max value
    prd.depth        = 0;

    optix::Ray ray(
                   ray_origin,
                   ray_direction,
                   radiance_ray_type,
                   scene_epsilon
                   );

    rtTrace( top_object, ray, prd );

    totalRadiance += prd.radiance;

  }

  totalRadiance /= sqrt_num_samples * sqrt_num_samples;

  output_buffer[ launch_index ] = make_float4( totalRadiance, 1.0 );

} // pinhole_camera



/////////////////////////////////////////////////////////
/// \brief pinhole_camera
/////////////////////////////////////////////////////////
RT_PROGRAM
void
pathtrace_pinhole_camera( )
{

  size_t2 screenSize = output_buffer.size( );

  float2 inv_screen  = 1.0f / make_float2( screenSize ) * 2.0f;
  float2 pixelCorner = make_float2( launch_index ) * inv_screen - 1.0f;

  float2 jitter_scale = inv_screen / sqrt_num_samples;

  float3 totalRadiance = make_float3( 0.0f );

  // loop vars
  unsigned x, y;
  float2 jitter;

  unsigned seed = tea< 16 >( screenSize.x * launch_index.y + launch_index.x, frame_number );
  seed += globalSeed;

  unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;

  // seems faster than two for loops for x and y on gpu
  while ( samples_per_pixel-- )
  {

    x = samples_per_pixel % sqrt_num_samples;
    y = samples_per_pixel / sqrt_num_samples;

    // random jitter within sample area
    jitter = make_float2( x + rnd( seed ), y + rnd( seed ) );

    float2 d             = pixelCorner + jitter * jitter_scale;
    float3 ray_origin    = eye;
    float3 ray_direction = normalize( d.x * U + d.y * V + W );

    PerRayData_pathtrace prd;
    prd.result       = make_float3( 0.f );
    prd.attenuation  = make_float3( 1.f );
    prd.radiance     = make_float3( 0.f );
    prd.countEmitted = true;
    prd.done         = false;
    prd.inside       = false;
    prd.seed         = seed;
    prd.depth        = 0;

    for ( ; ; )
    {

      float3 attenuation = prd.attenuation;

      optix::Ray ray(
                     ray_origin,
                     ray_direction,
                     radiance_ray_type,
                     scene_epsilon
                     );

      rtTrace( top_object, ray, prd );


      if ( prd.depth >= max_bounces )
      {

        prd.result += prd.radiance * attenuation;
        break;

      }

      if ( prd.depth >= first_bounce )
      {

        prd.result += prd.radiance * attenuation;

      }

      if ( prd.done )
      {

        break;

      }

      ++prd.depth;
      ray_origin    = prd.origin;
      ray_direction = prd.direction;

    }

    totalRadiance += prd.result;

  }

  totalRadiance /= sqrt_num_samples * sqrt_num_samples;

  if ( frame_number > 1 )
  {

    float a            = 1.0f / static_cast< float >( frame_number );
    float b            = ( static_cast< float >( frame_number ) - 1.0f ) * a;
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

  float2 inv_screen  = 1.0f / make_float2( screenSize ) * 2.0f;
  float2 pixelCorner = make_float2( launch_index ) * inv_screen - 1.0f;

  float2 jitter_scale = inv_screen / sqrt_num_samples;

  float3 totalRadiance = make_float3( 0.0f );

  // loop vars
  unsigned x, y;
  float2 jitter;

  unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;

  // seems faster than two for loops for x and y on gpu
  while ( samples_per_pixel-- )
  {

    x = samples_per_pixel % sqrt_num_samples;
    y = samples_per_pixel / sqrt_num_samples;

    // no random jitter, just center value
    jitter = make_float2( x + 0.5f, y + 0.5f );

    float2 d             = pixelCorner + jitter * jitter_scale;
    float3 ray_origin    = eye + d.x * U + d.y * V; // eye + offset in film space
    float3 ray_direction = normalize( W );          // always parallel view direction

    PerRayData_pathtrace prd;
    prd.result       = make_float3( 0.f );
    prd.attenuation  = make_float3( 1.f );
    prd.radiance     = make_float3( 0.f );
    prd.countEmitted = true;
    prd.done         = false;
    prd.inside       = false;
    prd.seed         = static_cast< unsigned >( -1 ); // overflow to max value
    prd.depth        = 0;

    optix::Ray ray(
                   ray_origin,
                   ray_direction,
                   radiance_ray_type,
                   scene_epsilon
                   );

    rtTrace( top_object, ray, prd );

    totalRadiance += prd.radiance;

  }

  totalRadiance /= sqrt_num_samples * sqrt_num_samples;

  output_buffer[ launch_index ] = make_float4( totalRadiance, 1.0 );

} // orthographic_camera



/////////////////////////////////////////////////////////
/// \brief pathtrace_orthographic_camera
/////////////////////////////////////////////////////////
RT_PROGRAM
void
pathtrace_orthographic_camera( )
{

  size_t2 screenSize = output_buffer.size( );

  float2 inv_screen  = 1.0f / make_float2( screenSize ) * 2.0f;
  float2 pixelCorner = make_float2( launch_index ) * inv_screen - 1.0f;

  float2 jitter_scale = inv_screen / sqrt_num_samples;

  float3 totalRadiance = make_float3( 0.0f );

  // loop vars
  unsigned x, y;
  float2 jitter;

  unsigned seed = tea< 16 >( screenSize.x * launch_index.y + launch_index.x, frame_number );
  seed += globalSeed;

  unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;

  // seems faster than two for loops for x and y on gpu
  while ( samples_per_pixel-- )
  {

    x = samples_per_pixel % sqrt_num_samples;
    y = samples_per_pixel / sqrt_num_samples;

    // random jitter within sample area
    jitter = make_float2( x + rnd( seed ), y + rnd( seed ) );

    float2 d             = pixelCorner + jitter * jitter_scale;
    float3 ray_origin    = eye + d.x * U + d.y * V; // eye + offset in film space
    float3 ray_direction = normalize( W );          // always parallel view direction


    PerRayData_pathtrace prd;
    prd.result       = make_float3( 0.f );
    prd.attenuation  = make_float3( 1.f );
    prd.radiance     = make_float3( 0.f );
    prd.countEmitted = true;
    prd.done         = false;
    prd.inside       = false;
    prd.seed         = seed;
    prd.depth        = 0;

    for ( ; ; )
    {

      float3 attenuation = prd.attenuation;

      optix::Ray ray(
                     ray_origin,
                     ray_direction,
                     radiance_ray_type,
                     scene_epsilon
                     );

      rtTrace( top_object, ray, prd );


      if ( prd.depth > max_bounces )
      {

        prd.result += prd.radiance * attenuation;
        break;

      }

      if ( prd.depth >= first_bounce )
      {

        prd.result += prd.radiance * attenuation;

      }

      if ( prd.done )
      {

        break;

      }

      ++prd.depth;
      ray_origin    = prd.origin;
      ray_direction = prd.direction;

    }

    totalRadiance += prd.result;

  }

  totalRadiance /= sqrt_num_samples * sqrt_num_samples;

  if ( frame_number > 1 )
  {

    float a            = 1.0f / static_cast< float >( frame_number );
    float b            = ( static_cast< float >( frame_number ) - 1.0f ) * a;
    float3 oldRadiance = make_float3( output_buffer[ launch_index ] );
    output_buffer[ launch_index ] = make_float4( a * totalRadiance + b * oldRadiance, 1.0f );

  }
  else
  {

    output_buffer[ launch_index ] = make_float4( totalRadiance, 1.0f );

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

  prd_current.radiance = bg_color;
  prd_current.done     = true;

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
