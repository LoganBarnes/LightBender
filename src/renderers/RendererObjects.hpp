
#include "optix.h"
#include "optix_math.h"
// Used by all the tutorial cuda files
//#include "commonStructs.h"

using namespace optix;

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

};

