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
/// \brief OptixBoxScene::_buildGeometry
///////////////////////////////////////////////////////////////
void
OptixBoxScene::_buildGeometry( )
{

  // Create primitives used in the scene
  optix::Geometry boxPrim    = createBoxPrimitive( );
  optix::Geometry spherePrim = createSpherePrimitive( );
  optix::Geometry quadPrim   = createQuadPrimitive( );

  // top group everything will get attached to
  optix::Group topGroup = context_->createGroup( );
  topGroup->setChildCount( 3 );

  // attach materials to geometries
  optix::GeometryGroup boxGroup = createGeomGroup(
                                                  { boxPrim },
                                                  { sceneMaterial_ },
                                                  "NoAccel",
                                                  "NoAccel"
                                                  );

  optix::GeometryGroup quadGroup = createGeomGroup(
                                                   { quadPrim },
                                                   { sceneMaterial_ },
                                                   "NoAccel",
                                                   "NoAccel"
                                                   );

  optix::GeometryGroup sphereGroup = createGeomGroup(
                                                     { spherePrim },
                                                     { sceneMaterial_ },
                                                     "NoAccel",
                                                     "NoAccel"
                                                     );


  attachToGroup( topGroup, boxGroup, 0, optix::make_float3( -3.0f, 0.0f, 0.0f ) );

  topGroup->setChild( 1, quadGroup );

  attachToGroup( topGroup, sphereGroup, 2, optix::make_float3( 3.0f, 0.0f, 0.0f ) );

  topGroup->setAcceleration( context_->createAcceleration( "NoAccel", "NoAccel" ) );


  context_[ "top_object"   ]->set( topGroup );
  context_[ "top_shadower" ]->set( topGroup );

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
