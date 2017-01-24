#pragma once

#include "optix.h"
#include "optix_math.h"
// Used by all the tutorial cuda files
//#include "commonStructs.h"


#ifdef FLT_MAX
#undef FLT_MAX
#endif

#define FLT_MAX         1e30;

static __device__ __inline__ float3 exp( const float3& x )
{
  return make_float3(exp(x.x), exp(x.y), exp(x.z));
}

static __device__ __inline__ float step( float min, float value )
{
  return value<min?0:1;
}

static __device__ __inline__ float3 mix( float3 a, float3 b, float x )
{
  return a*(1-x) + b*x;
}

static __device__ __inline__ float3 schlick( float nDi, const float3& rgb )
{
  float r = fresnel_schlick(nDi, 5, rgb.x, 1);
  float g = fresnel_schlick(nDi, 5, rgb.y, 1);
  float b = fresnel_schlick(nDi, 5, rgb.z, 1);
  return make_float3(r, g, b);
}

static __device__ __inline__ uchar4 make_color(const float3& c)
{
  return make_uchar4( static_cast<unsigned char>(__saturatef(c.z)*255.99f),  /* B */
                      static_cast<unsigned char>(__saturatef(c.y)*255.99f),  /* G */
                      static_cast<unsigned char>(__saturatef(c.x)*255.99f),  /* R */
                      255u);                                                 /* A */
}





static
__device__ __inline__
float3
refract(
        float3 I,
        float3 N
        )
{

  return I - 2.0 * dot( N, I ) * N;

}



static
__device__ __inline__
float3
refract(
        float3 I,
        float3 N,
        float  eta
        )
{

  float nDotI = dot( N, I );

  float k = 1.0f - eta * eta * ( 1.0f - nDotI * nDotI );

  if ( k < 0.0 )
  {
      return make_float3( 0.0f );       // or genDType(0.0)
  }
  else
  {
    return eta * I - ( eta * nDotI + sqrt( k ) ) * N;
  }

}



static
__device__ __inline__
float3
fresnel(
        float3 cosI,
        float3 cosT,
        float3 n1,
        float3 n2
        )
{

  float3 n1CosI = n1 * cosI;
  float3 n2CosT = n2 * cosT;

  float3 n1CosT = n1 * cosT;
  float3 n2CosI = n2 * cosI;

  float3 Rs = ( n1CosI - n2CosT ) / ( n1CosI + n2CosT );
  Rs *= Rs;

  float3 Rp = ( n1CosT - n2CosI ) / ( n1CosT + n2CosI );
  Rp *= Rp;

  return ( Rs + Rp ) * 0.5f;

}


struct PerRayData_radiance
{
  float3 result;
  float  importance;
  int depth;
};

struct PerRayData_shadow
{
  float3 attenuation;
};



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
  int useSpecular;

};

