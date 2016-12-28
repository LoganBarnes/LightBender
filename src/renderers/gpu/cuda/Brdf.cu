#include "optix.h"
#include "RendererObjects.hpp"
#include "commonStructs.h"


rtDeclareVariable( float3, shading_normal,   attribute shading_normal, );
rtDeclareVariable( float3, geometric_normal, attribute geometric_normal, );

rtDeclareVariable( PerRayData_radiance, prd_radiance, rtPayload, );


rtDeclareVariable( optix::Ray, ray,   rtCurrentRay, );
rtDeclareVariable( float,      t_hit, rtIntersectionDistance, );

rtBuffer< BasicLight > lights;


//
// Returns shading normal as the surface shading result
//
RT_PROGRAM
void
closest_hit_normals( )
{

  float3 world_geo_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 world_shade_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 ffnormal           = faceforward( world_shade_normal, -ray.direction, world_geo_normal );

  prd_radiance.result = ffnormal * 0.5f + 0.5f;

}



RT_PROGRAM
void
closest_hit_simple_shading( )
{

  float3 world_geo_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );
  float3 world_shade_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 ffnormal           = faceforward( world_shade_normal, -ray.direction, world_geo_normal );

  float3 radiance = make_float3( 0.0f );

  float3 hit_point = ray.origin + t_hit * ray.direction;

  for( int i = 0; i < lights.size( ); ++i )
  {

    BasicLight &light = lights[ i ];

    // direction and distance to light
    float3 w_i             = light.pos - hit_point;
    float  distToLightSqrd = dot( w_i, w_i );
    w_i                   /= sqrt( distToLightSqrd );

    float  cosAngle = max( 0.0, dot( ffnormal, w_i ) );

    radiance += cosAngle * light.radiance / distToLightSqrd;

  }

  prd_radiance.result = radiance;

}
