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
  , sceneMaterial_( context_->createMaterial( ) )
{


  // Materials
  std::string brdfPtxFile(
                          light::RES_PATH
                          + "ptx/cudaLightBender_generated_Brdf.cu.ptx"
                          );

  displayPrograms_.push_back( context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 "closest_hit_normals"
                                                                 ) );

  displayPrograms_.push_back( context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 "closest_hit_simple_shading"
                                                                 ) );

  displayPrograms_.push_back( context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 "closest_hit_bsdf"
                                                                 ) );

  sceneMaterial_->setClosestHitProgram( 0, displayPrograms_.back( ) );

  // for shadowing
  sceneMaterial_->setAnyHitProgram( 1, context_->createProgramFromPTXFile(
                                                                          brdfPtxFile,
                                                                          "any_hit_occlusion"
                                                                          ) );

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::~OptixScene
///////////////////////////////////////////////////////////////
OptixScene::~OptixScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::setDisplayType
/// \param type
///////////////////////////////////////////////////////////////
void
OptixScene::setDisplayType( int type )
{

  sceneMaterial_->setClosestHitProgram( 0, displayPrograms_[ static_cast< size_t >( type ) ] );

  resetFrameCount( );

}



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



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createGeomGroup
/// \param geometries
/// \param materials
/// \param builderAccel
/// \param traverserAccel
/// \return
///////////////////////////////////////////////////////////////
optix::GeometryGroup
OptixScene::createGeomGroup(
                            const std::vector< optix::Geometry > &geometries,
                            const std::vector< optix::Material > &materials,
                            const std::string                    &builderAccel,
                            const std::string                    &traverserAccel
                            )
{

  // check for one to one mapping of geometries and materials
  if ( materials.size( ) != geometries.size( ) )
  {

    throw std::runtime_error( "Geometries and Materials must contain the same number of elements" );

  }

  unsigned numInstances = static_cast< unsigned >( geometries.size( ) );

  // fill out geometry group for all geometries and materials
  optix::GeometryGroup geometryGroup = context_->createGeometryGroup( );
  geometryGroup->setChildCount( numInstances );

  for ( unsigned i = 0; i < numInstances; ++i )
  {

    optix::GeometryInstance gi = context_->createGeometryInstance( );

    gi->setGeometry( geometries[ i ] );
    gi->setMaterialCount( 1 );
    gi->setMaterial( 0, materials[ i ] );

    geometryGroup->setChild( i, gi );

  }

  geometryGroup->setAcceleration( context_->createAcceleration(
                                                               builderAccel.c_str( ),
                                                               traverserAccel.c_str( )
                                                               ) );

  return geometryGroup;

} // OptixScene::createGeomGroup



///////////////////////////////////////////////////////////////
/// \brief OptixScene::attachToGroup
/// \param group
/// \param geomGroup
/// \param translation
/// \param scale
/// \param rotationAngle
/// \param rotationAxis
///////////////////////////////////////////////////////////////
void
OptixScene::attachToGroup(
                          optix::Group         group,
                          optix::GeometryGroup geomGroup,
                          unsigned             childNum,
                          optix::float3        translation,
                          optix::float3        scale,
                          float                rotationAngle,
                          optix::float3        rotationAxis
                          )
{

  optix::Transform trans = context_->createTransform( );
  trans->setChild( geomGroup );
  group->setChild( childNum, trans );

  optix::Matrix4x4 T = optix::Matrix4x4::translate( translation );
  optix::Matrix4x4 S = optix::Matrix4x4::scale( scale );
  optix::Matrix4x4 R = optix::Matrix4x4::rotate( rotationAngle, rotationAxis );

  optix::Matrix4x4 M = T * R * S;

  trans->setMatrix( false, M.getData( ), 0 );

}



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
