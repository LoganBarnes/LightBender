#include "optix.h"
#include "RendererObjects.hpp"
#include "commonStructs.h"


rtDeclareVariable( float3,              shading_normal,   attribute shading_normal, );
rtDeclareVariable( float3,              geometric_normal, attribute geometric_normal, );

rtDeclareVariable( PerRayData_radiance, prd_radiance,     rtPayload, );
rtDeclareVariable( PerRayData_shadow,   prd_shadow,       rtPayload, );

rtDeclareVariable( optix::Ray,          ray,              rtCurrentRay, );
rtDeclareVariable( float,               t_hit,            rtIntersectionDistance, );


rtDeclareVariable( unsigned int,        shadow_ray_type,  , );
rtDeclareVariable( float,               scene_epsilon,    , );
rtDeclareVariable( rtObject,            top_object,       , );

rtBuffer< BasicLight > lights;



/////////////////////////////////////////////////////////
/// \brief closest_hit_normals
///
///        Sets shading normal as the surface color
/////////////////////////////////////////////////////////
RT_PROGRAM
void
closest_hit_normals( )
{

  float3 worldGeoNormal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 worldShadeNormal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 ffnormal         = faceforward( worldShadeNormal, -ray.direction, worldGeoNormal );

  prd_radiance.result = ffnormal * 0.5f + 0.5f;

}



/////////////////////////////////////////////////////////
/// \brief closest_hit_simple_shading
/////////////////////////////////////////////////////////
RT_PROGRAM
void
closest_hit_simple_shading( )
{

  float3 worldGeoNormal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 worldShadeNormal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 ffnormal         = faceforward( worldShadeNormal, -ray.direction, worldGeoNormal );

  float3 radiance = make_float3( 0.0f );

  float3 hit_point = ray.origin + t_hit * ray.direction;

  for ( int i = 0; i < lights.size( ); ++i )
  {

    BasicLight &light = lights[ i ];

    // direction and distance to light
    float3 w_i            = light.pos - hit_point;
    float distToLightPow2 = dot( w_i, w_i );
    float distToLight     = sqrt( distToLightPow2 );
    w_i /= distToLight; // normalizes w_i

    float cosAngle = dot( ffnormal, w_i );

    if ( cosAngle > 0.0f )
    {

      // results from shadow ray
      PerRayData_shadow shadow_prd;
      shadow_prd.attenuation = optix::make_float3( 1.0f );

      // shadow ray
      optix::Ray shadow_ray(
                            hit_point,
                            w_i,
                            shadow_ray_type,
                            scene_epsilon,
                            distToLight
                            );

      // shoot ray into scene
      rtTrace( top_object, shadow_ray, shadow_prd );


      radiance += ( 1.0 / M_PIf )                        // lambertian pi normalization
                  * ( light.radiance / distToLightPow2 ) // incident radiance
                  * cosAngle                             // angle between normal and incident ray
                  * shadow_prd.attenuation;              // attenuation from shadowing objects

    }

  }

  prd_radiance.result = radiance;

} // closest_hit_simple_shading



//rtDeclareVariable( float3, albedo,    , );
//rtDeclareVariable( float,  roughness, , );

/////////////////////////////////////////////////////////
/// \brief closest_hit_bsdf
/////////////////////////////////////////////////////////
RT_PROGRAM
void
closest_hit_bsdf( )
{

  float3 albedo   = make_float3( 0.71f, 0.62f, 0.53f );
  float roughness = 0.3f;

  float3 worldGeoNormal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 worldShadeNormal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 ffnormal         = faceforward( worldShadeNormal, -ray.direction, worldGeoNormal );

  float3 radiance = make_float3( 0.0f );

  float3 hit_point = ray.origin + t_hit * ray.direction;

  float3 w_o = -ray.direction;

  for ( int i = 0; i < lights.size( ); ++i )
  {

    BasicLight &light = lights[ i ];

    // direction and distance to light
    float3 w_i            = light.pos - hit_point;
    float distToLightPow2 = dot( w_i, w_i );
    float distToLight     = sqrt( distToLightPow2 );
    w_i /= distToLight; // normalizes w_i

    float cosAngle = dot( ffnormal, w_i );

    float3 localRadiance;

    if ( cosAngle > 0.0f )
    {

      // results from shadow ray
      PerRayData_shadow shadow_prd;
      shadow_prd.attenuation = optix::make_float3( 1.0f );

      // shadow ray
      optix::Ray shadow_ray(
                            hit_point,
                            w_i,
                            shadow_ray_type,
                            scene_epsilon,
                            distToLight
                            );

      // shoot ray into scene
      rtTrace( top_object, shadow_ray, shadow_prd );


      // only missing bsdf calculation right now
      localRadiance = ( light.radiance / distToLightPow2 ) // incident radiance
                      * cosAngle                           // angle between normal and incident ray
                      * shadow_prd.attenuation;            // attenuation from shadowing objects

      if ( dot( localRadiance, localRadiance ) > 1.0e-9f )
      {

        // brdf calculation

        // oren nayar diffuse brdf
        float gammaPow2 = roughness * roughness;

        float nDotL = optix::dot( ffnormal, w_i );
        float nDotV = optix::dot( ffnormal, w_o );

        float s = optix::dot( w_i, w_o ) - nDotL * nDotV;

        float t = s <= 0.0f ? 1.0f : max( nDotL, nDotV );

        float3 A = ( 1.0
                    - 0.5  * ( gammaPow2 / ( gammaPow2 + 0.33 ) )
                    + 0.17 * ( gammaPow2 / ( gammaPow2 + 0.13 ) ) * albedo
                    ) / M_PIf;

        float B = 0.45f * ( gammaPow2 / ( gammaPow2 + 0.09f ) ) / M_PIf;

        float3 onBrdf = albedo * ( A + B * s / t );

        radiance += localRadiance * onBrdf;

      }

    }

  }

  prd_radiance.result = radiance;

} // closest_hit_bsdf



/////////////////////////////////////////////////////////
/// \brief any_hit_occlusion
/////////////////////////////////////////////////////////
RT_PROGRAM
void
any_hit_occlusion( )
{

  // always opaque for now
  prd_shadow.attenuation = optix::make_float3( 0.0f );

  rtTerminateRay( );

}
