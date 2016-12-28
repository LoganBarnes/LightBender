#include "OptixBoxScene.hpp"
#include <optixu/optixu_math_stream_namespace.h>
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "commonStructs.h"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::OptixBoxScene
///////////////////////////////////////////////////////////////
OptixBoxScene::OptixBoxScene(
                             int      width,
                             int      height,
                             unsigned vbo
                             )
  : OptixRenderer( width, height, vbo )

{

  _buildGeometry( );
  _addLights( );

  context_->validate( );
  context_->compile( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::~OptixBoxScene
///////////////////////////////////////////////////////////////
OptixBoxScene::~OptixBoxScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::setDisplayType
/// \param type
///////////////////////////////////////////////////////////////
void
OptixBoxScene::setDisplayType( int type )
{

  sceneMaterial_->setClosestHitProgram( 0, materialPrograms_[ static_cast< size_t >( type ) ] );

}



///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::_buildGeometry
///////////////////////////////////////////////////////////////
void
OptixBoxScene::_buildGeometry( )
{

  // Create box
  std::string box_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Box.cu.ptx" );
  optix::Program box_bounds    = context_->createProgramFromPTXFile( box_ptx, "box_bounds" );
  optix::Program box_intersect = context_->createProgramFromPTXFile( box_ptx, "box_intersect" );

  optix::Geometry box = context_->createGeometry( );
  box->setPrimitiveCount( 1u );
  box->setBoundingBoxProgram( box_bounds );
  box->setIntersectionProgram( box_intersect );
  box[ "boxmin" ]->setFloat( -2.0f, -1.0f, -1.0f );
  box[ "boxmax" ]->setFloat( -4.0f,  1.0f,  1.0f );

  // Create sphere
  std::string sphere_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Sphere.cu.ptx" );
  optix::Program sphere_bounds    = context_->createProgramFromPTXFile( sphere_ptx, "bounds" );
  optix::Program sphere_intersect = context_->createProgramFromPTXFile( sphere_ptx, "intersect" );

  optix::Geometry sphere = context_->createGeometry( );
  sphere->setPrimitiveCount( 1u );
  sphere->setBoundingBoxProgram ( sphere_bounds );
  sphere->setIntersectionProgram( sphere_intersect );
  sphere[ "sphere" ]->setFloat( 3.0f, 0.0f, 0.0f, 1.0f );

  // Create quad
  std::string quad_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Parallelogram.cu.ptx" );
  optix::Program quad_bounds    = context_->createProgramFromPTXFile( quad_ptx, "bounds" );
  optix::Program quad_intersect = context_->createProgramFromPTXFile( quad_ptx, "intersect" );

  optix::Geometry quad = context_->createGeometry( );
  quad->setPrimitiveCount( 1u );
  quad->setBoundingBoxProgram( quad_bounds );
  quad->setIntersectionProgram( quad_intersect );

  optix::float3 anchor = optix::make_float3( -1, -1,  0 );
  optix::float3 v1     = optix::make_float3(  2,  0,  0 );
  optix::float3 v2     = optix::make_float3(  0,  2,  0 );
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

  // Materials
  std::string brdfPtxFile(
                          light::RES_PATH
                          + "ptx/cudaLightBender_generated_Brdf.cu.ptx"
                          );

  sceneMaterial_ = context_->createMaterial( );

  materialPrograms_.push_back( context_->createProgramFromPTXFile(
                                                                  brdfPtxFile,
                                                                  "closest_hit_normals"
                                                                  ) );

  materialPrograms_.push_back( context_->createProgramFromPTXFile(
                                                                  brdfPtxFile,
                                                                  "closest_hit_simple_shading"
                                                                  ) );

  sceneMaterial_->setClosestHitProgram( 0, materialPrograms_.back( ) );


  // Create GIs for each piece of geometry
  std::vector< optix::GeometryInstance > gis;
  gis.push_back( context_->createGeometryInstance( box,    &sceneMaterial_, &sceneMaterial_ + 1 ) );
  gis.push_back( context_->createGeometryInstance( quad,   &sceneMaterial_, &sceneMaterial_ + 1 ) );
  gis.push_back( context_->createGeometryInstance( sphere, &sceneMaterial_, &sceneMaterial_ + 1 ) );

  // Place all in group
  optix::GeometryGroup geometrygroup = context_->createGeometryGroup( );
  geometrygroup->setChildCount( static_cast< unsigned int >( gis.size( ) ) );

  for ( unsigned i = 0; i < gis.size( ); ++i )
  {

    geometrygroup->setChild( i, gis[ i ] );

  }

  geometrygroup->setAcceleration( context_->createAcceleration( "NoAccel", "NoAccel" ) );

  context_[ "top_object" ]->set( geometrygroup );

} // OptixBoxScene::_buildGeometry



///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::_addLights
///////////////////////////////////////////////////////////////
void
OptixBoxScene::_addLights( )
{

  std::vector< BasicLight > lights = {
    { optix::make_float3(  10.0f, 30.0f, 20.0f ),
      optix::make_float3( 500.0f, 500.0f, 500.0f ),
      1, 0 },

    { optix::make_float3( -10.0f, 20.0f, 15.0f ),
      optix::make_float3( 300.0f, 300.0f, 300.0f ),
      1, 0 },

    { optix::make_float3( 0.0f, -10.0f, -25.0f ),
      optix::make_float3( 100.0f, 100.0f, 100.0f ),
      1, 0 }
  };

  optix::Buffer lightBuffer = context_->createBuffer( RT_BUFFER_INPUT );

  lightBuffer->setFormat( RT_FORMAT_USER );
  lightBuffer->setElementSize( sizeof( lights[ 0 ] ) );
  lightBuffer->setSize( lights.size( ) );
  memcpy( lightBuffer->map( ), lights.data( ), lights.size( ) * sizeof( lights[ 0 ] ) );
  lightBuffer->unmap( );

  context_[ "lights" ]->set( lightBuffer );

} // OptixBoxScene::_addLights



} // namespace light
