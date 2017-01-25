#include <optix.h>
#include <optixu/optixu_math_stream_namespace.h>
#include "commonStructs.h"
#include "random.h"
#include "RendererObjects.hpp" // should be last to avoid FLT_MAX redefintion warning



//////////////////////////////////////////////////////////////
/// \brief createONB
///
///        Create Orthonormal Basis from normalized vector
//////////////////////////////////////////////////////////////
static
__device__ __inline__
void
createONB(
          const float3 &n, ///< normal
          float3       &U, ///< output U vector
          float3       &V  ///< output V vector
          )
{

  U = cross( n, make_float3( 0.0f, 1.0f, 0.0f ) );

  if ( dot( U, U ) < 1.e-3f )
  {

    U = cross( n, make_float3( 1.0f, 0.0f, 0.0f ) );

  }

  U = normalize( U );
  V = cross( n, U );

}



//////////////////////////////////////////////////////////////
/// \brief sampleLight
///
///        Choose a random point from a spherical light
///
/// \return
//////////////////////////////////////////////////////////////
static
__device__ __inline__
float3
sampleIlluminator(
                  unsigned             &seed,        ///< random seed
                  const SurfaceElement &surfel,      ///< info about the current surface
                  const Illuminator    &illuminator, ///< info about the curren illuminator
                  float               *pPdf          ///< output pdf value
                  )
{

  float theta = rnd( seed ) * 2 * M_PIf;
  float u     = rnd( seed ) * 2.0 - 1.0;

  float xyCoeff = sqrt( 1.0 - u * u );

  float3 samplePos = make_float3(
                                 xyCoeff * cos( theta ),
                                 xyCoeff * sin( theta ),
                                 u
                                 );

  // sample on hemisphere in direction of point
  if ( dot( samplePos, normalize( surfel.point - illuminator.center ) ) < 0.0f )
  {

    samplePos = -samplePos;

  }

  *pPdf = M_PIf;

  samplePos = illuminator.center + samplePos * illuminator.radius;

  return samplePos;

} // sampleIlluminator



static
__device__ __inline__
float3
calculateSpecular(
                  const float3         &V,
                  const float3         &L,
                  const float3         &F,
                  const SurfaceElement &surfel
                  )
{

  // roughness -> 'm' in cook-torrance lingo
  float m = surfel.material.roughness;

  float3 H = normalize( V + L );

  float cosNV = dot( surfel.normal, V );
  float cosNH = dot( surfel.normal, H );
  float cosNL = dot( surfel.normal, L );
  float cosVH = dot( V, H );

  // geometric attenuation
  float G = min( 1.0f, min( 2.0f * cosNH * cosNV / cosVH, 2.0f * cosNH * cosNL / cosVH ) );

  // microfacet slope distribution
  float cosNHPow2 = cosNH * cosNH;
  float mPo2      = m * m;

  float D = ( 1.0 / ( M_PIf * mPo2 * cosNHPow2 * cosNHPow2 ) )
            * exp( ( cosNHPow2 - 1.0 ) / ( mPo2 * cosNHPow2 ) );

  // return mat.albedo * ( F * D * G ) / ( PI * cosNL * cosNV );
  // return mat.albedo * F;
  float3 specular = surfel.material.albedo * ( F * D * G ) / ( M_PIf * cosNL * cosNV );

  float3 diffuse = surfel.material.albedo * ( 1.0 - F ) / M_PIf;

  return diffuse + specular;

} // calculateSpecular



rtDeclareVariable( float3,               shading_normal,   attribute shading_normal, );
rtDeclareVariable( float3,               geometric_normal, attribute geometric_normal, );

rtDeclareVariable( PerRayData_pathtrace, prd_current,      rtPayload, );
rtDeclareVariable( PerRayData_shadow,    prd_shadow,       rtPayload, );

rtDeclareVariable( optix::Ray,           ray,              rtCurrentRay, );
rtDeclareVariable( float,                t_hit,            rtIntersectionDistance, );


rtDeclareVariable( unsigned int,         shadow_ray_type,  , );
rtDeclareVariable( float,                scene_epsilon,    , );
rtDeclareVariable( rtObject,             top_shadower,     , );

rtBuffer< Illuminator > illuminators;



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

  prd_current.radiance = ffnormal * 0.5f + 0.5f;
  prd_current.done     = true;

}



/////////////////////////////////////////////////////////
/// \brief closest_hit_simple_shading
/////////////////////////////////////////////////////////
RT_PROGRAM
void
closest_hit_simple_shading( )
{

  const float3 simpleShadeAlbedo = make_float3( 0.8f );

  SurfaceElement surfel;

  float3 worldGeoNormal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 worldShadeNormal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );

  surfel.normal = faceforward( worldShadeNormal, -ray.direction, worldGeoNormal );

  float3 radiance = make_float3( 0.0f );

  surfel.point = ray.origin + t_hit * ray.direction;


  // loop vars
  float3 w_i;
  float distToLightPow2, distToLight;

  for ( int i = 0; i < illuminators.size( ); ++i )
  {

    Illuminator &illuminator = illuminators[ i ];

    float3 lightPos = illuminator.center;
    float3 flux     = illuminator.radiantFlux;

    float totalDistPow2;
    float pdf        = M_PIf;
    float mis_weight = 1.0f;

    // randomly sample sphere (only light shape for now)
    if ( prd_current.seed != static_cast< unsigned >( -1 ) )
    {

      lightPos = sampleIlluminator( prd_current.seed, surfel, illuminator, &pdf );
//      mis_weight = 0.5f;

      // direction and distance to light
      w_i             = lightPos - surfel.point;
      distToLightPow2 = dot( w_i, w_i );
      distToLight     = sqrt( distToLightPow2 );
      w_i            /= distToLight; // normalizes w_i

      totalDistPow2  = distToLight + illuminator.radius;
      totalDistPow2 *= totalDistPow2;

      // lambertian emitter
      ///\todo: Sample by sollid angle for quicker convergance
      flux *= 0.5f * max( 0.0, dot( -w_i, normalize( lightPos - illuminator.center ) ) );

    }
    else
    {

      // direction and distance to light
      w_i             = lightPos - surfel.point;
      distToLightPow2 = dot( w_i, w_i );
      distToLight     = sqrt( distToLightPow2 );
      w_i            /= distToLight; // normalizes w_i

      totalDistPow2  = distToLight;
      totalDistPow2 *= totalDistPow2;

      flux /= 4.0f;

    }


    float cosAngle = dot( surfel.normal, w_i );

    if ( cosAngle > 0.0f )
    {

      // results from shadow ray
      PerRayData_shadow shadow_prd;
      shadow_prd.attenuation = optix::make_float3( 1.0f );

      // shadow ray
      optix::Ray shadow_ray(
                            surfel.point,
                            w_i,
                            shadow_ray_type,
                            scene_epsilon,
                            distToLight
                            );

      // shoot ray into scene
      rtTrace( top_shadower, shadow_ray, shadow_prd );


      radiance += ( simpleShadeAlbedo / M_PIf ) // lambertian pi normalization
                  * ( flux / totalDistPow2 )    // inverse square law
                  * ( mis_weight / pdf )        // importance weight
                  * cosAngle                    // angle between normal and incident ray
                  * shadow_prd.attenuation;     // attenuation from shadowing objects

    }

  }


  //
  // next ray for indirect light
  //
  if ( prd_current.seed != static_cast< unsigned >( -1 ) )
  {

    float scatterProb = ( simpleShadeAlbedo.x + simpleShadeAlbedo.y + simpleShadeAlbedo.z ) / 3;

    float rouletteVal = rnd( prd_current.seed );

    //
    // scatter
    //
    rouletteVal -= scatterProb;

    if ( rouletteVal <= 0.0f )
    {

      prd_current.origin = surfel.point;

      float z1 = rnd( prd_current.seed );
      float z2 = rnd( prd_current.seed );
      float3 p;

      optix::cosine_sample_hemisphere( z1, z2, p );

      float3 v1, v2;
      createONB( surfel.normal, v1, v2 );

      prd_current.direction    = v1 * p.x + v2 * p.y + surfel.normal * p.z;
      prd_current.attenuation *= simpleShadeAlbedo / scatterProb;
      prd_current.countEmitted = false;

      prd_current.radiance = radiance;
      return;

    }

    //
    // absorb
    //
    prd_current.done = true;

  }

  prd_current.radiance = radiance;

} // closest_hit_simple_shading



// albedo:
// 0.13f // moon
// 0.71f, 0.62f, 0.53f // clay
// roughness = 0.3f;

rtDeclareVariable( float3, albedo,    , );
rtDeclareVariable( float,  roughness, , );
rtDeclareVariable( float3, ior,       , );

/////////////////////////////////////////////////////////
/// \brief closest_hit_bsdf
/////////////////////////////////////////////////////////
RT_PROGRAM
void
closest_hit_bsdf( )
{

//  float k = 0.5;

  SurfaceElement surfel;

  surfel.material.albedo    = albedo;
  surfel.material.roughness = roughness;
  surfel.material.IOR       = ior;

  float3 worldGeoNormal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 worldShadeNormal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );

  surfel.normal = faceforward( worldShadeNormal, -ray.direction, worldGeoNormal );

  float3 radiance = make_float3( 0.0f );

  surfel.point = ray.origin + t_hit * ray.direction;

  // view vector
  float3 w_v = -ray.direction;


  //
  // fresnel calculation for current surface
  //
  float3 currentIOR = make_float3( 1.0 ); // air (no transmission yet)

  float cosNV = dot( surfel.normal, w_v );

  float3 eta = currentIOR / surfel.material.IOR;
  float3 cosT;

  // individual fresnel calc for each RGB wavelength
  float3 T = refract( -w_v, surfel.normal, eta.x );
  cosT.x = dot( -surfel.normal, T );

  T      = refract( -w_v, surfel.normal, eta.y );
  cosT.y = dot( -surfel.normal, T );

  T      = refract( -w_v, surfel.normal, eta.z );
  cosT.z = dot( -surfel.normal, T );

  float3 F = fresnel( make_float3( cosNV ), cosT, currentIOR, surfel.material.IOR );



  // loop vars
  float3 w_l; // light vector
  float distToLightPow2, distToLight;

  for ( int i = 0; i < illuminators.size( ); ++i )
  {

    Illuminator &illuminator = illuminators[ i ];

    float3 lightPos = illuminator.center;
    float3 flux     = illuminator.radiantFlux;

    float totalDistPow2;
    float pdf        = M_PIf;
    float mis_weight = 1.0f;

    // randomly sample sphere (only light shape for now)
    if ( prd_current.seed != static_cast< unsigned >( -1 ) )
    {

      lightPos = sampleIlluminator( prd_current.seed, surfel, illuminator, &pdf );
//      mis_weight = 0.5;

      // direction and distance to light
      w_l             = lightPos - surfel.point;
      distToLightPow2 = dot( w_l, w_l );
      distToLight     = sqrt( distToLightPow2 );
      w_l            /= distToLight; // normalizes w_i

      totalDistPow2  = distToLight + illuminator.radius;
      totalDistPow2 *= totalDistPow2;

      // lambertian emitter
      ///\todo: Sample by sollid angle for quicker convergance
      flux *= 0.5f * max( 0.0, dot( -w_l, normalize( lightPos - illuminator.center ) ) );

    }
    else
    {

      // direction and distance to light
      w_l             = lightPos - surfel.point;
      distToLightPow2 = dot( w_l, w_l );
      distToLight     = sqrt( distToLightPow2 );
      w_l            /= distToLight; // normalizes w_i

      totalDistPow2  = distToLight;
      totalDistPow2 *= totalDistPow2;

      flux /= 4.0f;

    }


    float cosNL = dot( surfel.normal, w_l );

    float3 localRadiance;

    if ( cosNL > 0.0f )
    {

      // results from shadow ray
      PerRayData_shadow shadow_prd;
      shadow_prd.attenuation = optix::make_float3( 1.0f );

      // shadow ray
      optix::Ray shadow_ray(
                            surfel.point,
                            w_l,
                            shadow_ray_type,
                            scene_epsilon,
                            distToLight
                            );

      // shoot ray into scene
      rtTrace( top_shadower, shadow_ray, shadow_prd );


      // bsdf calculation added below
      localRadiance = ( flux / totalDistPow2 )  // incident radiance
                      * ( mis_weight / pdf )    // importance weighting
                      * cosNL                   // angle between normal and incident ray
                      * shadow_prd.attenuation; // attenuation from shadowing objects


      if ( dot( localRadiance, localRadiance ) > 1.0e-9f )
      {

        //
        // bsdf calculation
        //

        //
        // cook-torrance specular
        //
        float3 specular = make_float3( 0.0f );

        if ( prd_current.useSpecular )
        {

          specular = calculateSpecular( w_v, w_l, F, surfel );

        }

        //
        // oren nayar diffuse brdf
        //
        float gammaPow2 = surfel.material.roughness * surfel.material.roughness;

        float nDotL = optix::dot( surfel.normal, w_l );
        float nDotV = optix::dot( surfel.normal, w_v );

        float s = optix::dot( w_l, w_v ) - nDotL * nDotV;

        float t = s <= 0.0f ? 1.0f : max( nDotL, nDotV );

        float3 A = ( 1.0
                    - 0.5  * ( gammaPow2 / ( gammaPow2 + 0.33 ) )
                    + 0.17 * ( gammaPow2 / ( gammaPow2 + 0.13 ) ) * surfel.material.albedo
                    ) / M_PIf;

        float B = 0.45f * ( gammaPow2 / ( gammaPow2 + 0.09f ) ) / M_PIf;

        float3 diffuse = surfel.material.albedo * ( A + B * s / t );

        radiance += localRadiance * ( diffuse * ( 1.0f - F ) + specular );

      }

    }

  }


  //
  // next ray for indirect light
  //
  if ( prd_current.seed != static_cast< unsigned >( -1 ) )
  {

    float reflectProb = ( F.x + F.y + F.z ) / 3;

    float scatterProb = ( albedo.x + albedo.y + albedo.z ) / 3;
//    scatterProb *= 1.0 - reflectProb;

    //
    // russian roulette based on scattering probabilities
    //
    float rouletteVal = rnd( prd_current.seed );

    //
    // diffuse scatter
    //
    rouletteVal -= scatterProb;

    if ( rouletteVal <= 0.0f )
    {

      prd_current.origin = surfel.point;

      float z1 = rnd( prd_current.seed );
      float z2 = rnd( prd_current.seed );
      float3 p;

      optix::cosine_sample_hemisphere( z1, z2, p );

      float3 v1, v2;
      createONB( surfel.normal, v1, v2 );

      prd_current.direction    = v1 * p.x + v2 * p.y + surfel.normal * p.z;
      prd_current.attenuation *= albedo / scatterProb;  // use the albedo as the diffuse response
      prd_current.countEmitted = false;
      prd_current.useSpecular  = false;

      prd_current.radiance = radiance;
      return;

    }


    //
    // reflect
    //
    rouletteVal -= reflectProb;

    if ( rouletteVal <= 0.0f )
    {

      //
      // sample from raised cosine distribution
      //
      float z1 = rnd( prd_current.seed );
      float z2 = rnd( prd_current.seed );
      float3 p;

      optix::cosine_sample_hemisphere( z1, z2, p );

      float scaling = roughness;

      p.x *= scaling;
      p.y *= scaling;

      p.z /= scaling;

      p = normalize( p );

      float3 v1, v2;
      float3 R = reflect( -w_v, surfel.normal );
      createONB( R, v1, v2 );

      prd_current.origin       = surfel.point;
      prd_current.direction    =  v1 * p.x + v2 * p.y + R * p.z;
      prd_current.attenuation *= F / reflectProb;
//      prd_current.countEmitted = true;

      prd_current.radiance = radiance;
      return;

    }


    //
    // absorb
    //
    prd_current.done = true;

  }

  prd_current.radiance = radiance;

} // closest_hit_bsdf



rtDeclareVariable( float3, emissionRadiance, , );

RT_PROGRAM
void
closest_hit_emission( )
{

  prd_current.radiance = prd_current.countEmitted ? emissionRadiance : make_float3( 0.f );
//  prd_current.radiance = emissionRadiance * prd_current.attenuation;
  prd_current.done = true;

}



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
