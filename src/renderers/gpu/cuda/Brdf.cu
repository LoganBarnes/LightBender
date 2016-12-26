#include "optix.h"
#include "RendererObjects.hpp"


rtDeclareVariable( float3,              shading_normal,    attribute shading_normal, );
rtDeclareVariable( PerRayData_radiance, prd_radiance,      rtPayload, );


//
// Returns shading normal as the surface shading result
//
RT_PROGRAM
void
closest_hit_normals( )
{

  prd_radiance.result = normalize( rtTransformNormal(
                                                     RT_OBJECT_TO_WORLD,
                                                     shading_normal
                                                     ) ) * 0.5f + 0.5f;
}
