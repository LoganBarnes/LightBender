#include "OptixBasicScene.hpp"
#include <optixu/optixu_math_stream_namespace.h>
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "commonStructs.h"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixBasicScene::OptixBasicScene
///////////////////////////////////////////////////////////////
OptixBasicScene::OptixBasicScene(
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
/// \brief OptixBasicScene::~OptixBasicScene
///////////////////////////////////////////////////////////////
OptixBasicScene::~OptixBasicScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixBasicScene::_buildGeometry
///////////////////////////////////////////////////////////////
void
OptixBasicScene::_buildGeometry( )
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

  context_[ "top_object" ]->set( topGroup );

} // OptixBasicScene::_buildGeometry



///////////////////////////////////////////////////////////////
/// \brief OptixBasicScene::_addLights
///////////////////////////////////////////////////////////////
void
OptixBasicScene::_addLights( )
{

  std::vector< BasicLight > lights = {
    { optix::make_float3(  10.0f, 30.0f, 20.0f ),
      optix::make_float3( 1500.0f ),
      1, 0 },

    { optix::make_float3( -10.0f, 20.0f, 15.0f ),
      optix::make_float3( 900.0f ),
      1, 0 },

    { optix::make_float3( 0.0f, -10.0f, -25.0f ),
      optix::make_float3( 200.0f ),
      1, 0 }
  };

  optix::Buffer lightBuffer = context_->createBuffer( RT_BUFFER_INPUT );

  lightBuffer->setFormat( RT_FORMAT_USER );
  lightBuffer->setElementSize( sizeof( lights[ 0 ] ) );
  lightBuffer->setSize( lights.size( ) );
  memcpy( lightBuffer->map( ), lights.data( ), lights.size( ) * sizeof( lights[ 0 ] ) );
  lightBuffer->unmap( );

  context_[ "lights" ]->set( lightBuffer );

} // OptixBasicScene::_addLights



} // namespace light
