#include "OptixBasicScene.hpp"
#include "optixMod/optix_math_stream_namespace_mod.h"
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

  context_[ "top_object"   ]->set( topGroup );
  context_[ "top_shadower" ]->set( topGroup );

} // OptixBasicScene::_buildGeometry



///////////////////////////////////////////////////////////////
/// \brief OptixBasicScene::_addLights
///////////////////////////////////////////////////////////////
void
OptixBasicScene::_addLights( )
{

  std::vector< Light > lights =
  {

    createLight(
                optix::make_float3(  2.0f, 6.0f, 4.0f ),
                optix::make_float3( 120.0f ),
                LightShape::SPHERE,
                0.1f
                )

  };

  context_[ "lights" ]->set( createInputBuffer( lights ) );

} // OptixBasicScene::_addLights



} // namespace light
