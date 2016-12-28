#include "OptixScene.hpp"
#include <optixu/optixu_math_stream_namespace.h>
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "commonStructs.h"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixScene::OptixScene
///////////////////////////////////////////////////////////////
OptixScene::OptixScene(
                       int      width,
                       int      height,
                       unsigned vbo
                       )
  : OptixRenderer( width, height, vbo )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::~OptixScene
///////////////////////////////////////////////////////////////
OptixScene::~OptixScene( )
{}



/////////////////////////////////////////////////////////////////
///// \brief OptixScene::setDisplayType
///// \param type
/////////////////////////////////////////////////////////////////
//void
//OptixScene::setDisplayType( int type )
//{

//  sceneMaterial_->setClosestHitProgram( 0, materialPrograms_[ static_cast< size_t >( type ) ] );

//}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createBoxPrimitive
/// \param min
/// \param max
/// \return
///////////////////////////////////////////////////////////////
optix::Geometry
OptixScene::createBoxPrimitive(
                               optix::float3 min,
                               optix::float3 max
                               )
{

  // Create box
  std::string box_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Box.cu.ptx" );
  optix::Program box_bounds    = context_->createProgramFromPTXFile( box_ptx, "box_bounds" );
  optix::Program box_intersect = context_->createProgramFromPTXFile( box_ptx, "box_intersect" );

  optix::Geometry box = context_->createGeometry( );
  box->setPrimitiveCount( 1u );
  box->setBoundingBoxProgram( box_bounds );
  box->setIntersectionProgram( box_intersect );
  box[ "boxmin" ]->setFloat( min );
  box[ "boxmax" ]->setFloat( max );

  return box;

} // OptixScene::createBoxPrimitive



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createSpherePrimitive
/// \param center
/// \param radius
/// \return
///////////////////////////////////////////////////////////////
optix::Geometry
OptixScene::createSpherePrimitive(
                                  optix::float3 center,
                                  float         radius
                                  )
{

  // Create sphere
  std::string sphere_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Sphere.cu.ptx" );
  optix::Program sphere_bounds    = context_->createProgramFromPTXFile( sphere_ptx, "bounds" );
  optix::Program sphere_intersect = context_->createProgramFromPTXFile( sphere_ptx, "intersect" );

  optix::Geometry sphere = context_->createGeometry( );
  sphere->setPrimitiveCount( 1u );
  sphere->setBoundingBoxProgram ( sphere_bounds );
  sphere->setIntersectionProgram( sphere_intersect );
  sphere[ "sphere" ]->setFloat( center.x, center.y, center.z, radius );

  return sphere;

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createQuadPrimitive
/// \param anchor
/// \param v1
/// \param v2
/// \return
///////////////////////////////////////////////////////////////
optix::Geometry
OptixScene::createQuadPrimitive(
                                optix::float3 anchor,
                                optix::float3 v1,
                                optix::float3 v2
                                )
{

  // Create quad
  std::string quad_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Parallelogram.cu.ptx" );
  optix::Program quad_bounds    = context_->createProgramFromPTXFile( quad_ptx, "bounds" );
  optix::Program quad_intersect = context_->createProgramFromPTXFile( quad_ptx, "intersect" );

  optix::Geometry quad = context_->createGeometry( );
  quad->setPrimitiveCount( 1u );
  quad->setBoundingBoxProgram( quad_bounds );
  quad->setIntersectionProgram( quad_intersect );

  optix::float3 normal = optix::cross( v1, v2 );

  normal = normalize( normal );

  float d = dot( normal, anchor );
  v1 *= 1.0f / dot( v1, v1 );
  v2 *= 1.0f / dot( v2, v2 );
  optix::float4 plane = optix::make_float4( normal, d );

  quad[ "plane"  ]->setFloat( plane.x, plane.y, plane.z, plane.w );
  quad[ "v1"     ]->setFloat( v1.x, v1.y, v1.z );
  quad[ "v2"     ]->setFloat( v2.x, v2.y, v2.z );
  quad[ "anchor" ]->setFloat( anchor.x, anchor.y, anchor.z );

  return quad;

} // OptixScene::createQuadPrimitive



// ///////////////////////////////////////////////////////////////
// /// \brief OptixScene::_addLights
// ///////////////////////////////////////////////////////////////
// void
// OptixScene::_addLights( )
// {

//   std::vector< BasicLight > lights = {
//     { optix::make_float3(  10.0f, 30.0f, 20.0f ),
//       optix::make_float3( 500.0f, 500.0f, 500.0f ),
//       1, 0 },

//     { optix::make_float3( -10.0f, 20.0f, 15.0f ),
//       optix::make_float3( 300.0f, 300.0f, 300.0f ),
//       1, 0 },

//     { optix::make_float3( 0.0f, -10.0f, -25.0f ),
//       optix::make_float3( 100.0f, 100.0f, 100.0f ),
//       1, 0 }
//   };

//   optix::Buffer lightBuffer = context_->createBuffer( RT_BUFFER_INPUT );

//   lightBuffer->setFormat( RT_FORMAT_USER );
//   lightBuffer->setElementSize( sizeof( lights[ 0 ] ) );
//   lightBuffer->setSize( lights.size( ) );
//   memcpy( lightBuffer->map( ), lights.data( ), lights.size( ) * sizeof( lights[ 0 ] ) );
//   lightBuffer->unmap( );

//   context_[ "lights" ]->set( lightBuffer );

// } // OptixScene::_addLights



} // namespace light
