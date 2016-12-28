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
  : OptixScene( width, height, vbo )

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
  optix::Geometry box = createBoxPrimitive(
                                           optix::      make_float3( -2.0f, -1.0f,   -1.0f ),
                                           optix::      make_float3( -4.0f, 1.0f, 1.0f )
                                           );

  // Create sphere
  optix::Geometry sphere = createSpherePrimitive(
                                                 optix::make_float3( 3.0f, 0.0f, 0.0f ),
                                                 1.0f
                                                 );

  // Create quad
  optix::Geometry quad = createQuadPrimitive(
                                             optix::make_float3( -1, -1, 0 ),
                                             optix::make_float3(  2, 0,  0 ),
                                             optix::make_float3(  0, 2,  0 )
                                             );

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
