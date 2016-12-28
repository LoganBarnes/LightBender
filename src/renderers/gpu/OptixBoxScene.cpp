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

  boxMaterial_->setClosestHitProgram( 0, materialPrograms_[ static_cast< size_t >( type ) ] );

}



///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::_buildGeometry
///////////////////////////////////////////////////////////////
void
OptixBoxScene::_buildGeometry( )
{

  std::string box_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Box.cu.ptx" );
  optix::Program box_bounds    = context_->createProgramFromPTXFile( box_ptx, "box_bounds" );
  optix::Program box_intersect = context_->createProgramFromPTXFile( box_ptx, "box_intersect" );

  // Create box
  optix::Geometry box = context_->createGeometry( );
  box->setPrimitiveCount( 1u );
  box->setBoundingBoxProgram( box_bounds );
  box->setIntersectionProgram( box_intersect );
  box[ "boxmin" ]->setFloat( -2.0f, -3.0f, -2.0f );
  box[ "boxmax" ]->setFloat(  2.0f,  4.0f,  2.0f );

  // Materials
  std::string brdfPtxFile(
                          light::RES_PATH
                          + "ptx/cudaLightBender_generated_Brdf.cu.ptx"
                          );

  boxMaterial_ = context_->createMaterial( );

  materialPrograms_.push_back( context_->createProgramFromPTXFile(
                                                                  brdfPtxFile,
                                                                  "closest_hit_normals"
                                                                  ) );

  materialPrograms_.push_back( context_->createProgramFromPTXFile(
                                                                  brdfPtxFile,
                                                                  "closest_hit_simple_shading"
                                                                  ) );

  boxMaterial_->setClosestHitProgram( 0, materialPrograms_.back( ) );


  // Create GIs for each piece of geometry
  std::vector< optix::GeometryInstance > gis;
  gis.push_back( context_->createGeometryInstance( box, &boxMaterial_, &boxMaterial_ + 1 ) );

  // Place all in group
  optix::GeometryGroup geometrygroup = context_->createGeometryGroup( );
  geometrygroup->setChildCount( static_cast< unsigned int >( gis.size( ) ) );
  geometrygroup->setChild( 0, gis[ 0 ] );
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
    { optix::make_float3(  10.0f, 30.0f, 20.0f ), optix::make_float3( 500.0f, 500.0f, 500.0f ), 1, 0 },
    { optix::make_float3( -10.0f, 20.0f, 15.0f ), optix::make_float3( 300.0f, 300.0f, 300.0f ), 1, 0 }
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
