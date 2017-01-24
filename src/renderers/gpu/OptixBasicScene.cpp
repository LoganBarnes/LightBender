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

  _buildScene( );

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
OptixBasicScene::_buildScene( )
{

  optix::float3 albedo = optix::make_float3( 0.71f, 0.62f, 0.53f );   // clay
  float roughness      = 0.3f;

  // Create primitives used in the scene
  optix::Geometry boxPrim    = createBoxPrimitive( );
  optix::Geometry quadPrim   = createQuadPrimitive( );
  optix::Geometry spherePrim = createSpherePrimitive( );

  // Create materials used in the scene
  optix::Material boxMaterial = createMaterial(
                                               materialPrograms_[ "closest_hit_bsdf" ],
                                               materialPrograms_[ "any_hit_occlusion" ]
                                               );
  optix::Material quadMaterial = createMaterial(
                                                materialPrograms_[ "closest_hit_bsdf" ],
                                                materialPrograms_[ "any_hit_occlusion" ]
                                                );
  optix::Material sphereMaterial = createMaterial(
                                                  materialPrograms_[ "closest_hit_bsdf" ],
                                                  materialPrograms_[ "any_hit_occlusion" ]
                                                  );

  boxMaterial   [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  quadMaterial  [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  sphereMaterial[ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );

  boxMaterial   [ "roughness" ]->setFloat( roughness );
  quadMaterial  [ "roughness" ]->setFloat( roughness );
  sphereMaterial[ "roughness" ]->setFloat( roughness );

  boxMaterial   [ "indexOfRefraction" ]->setFloat( 1.5f, 1.5f, 1.5f );
  quadMaterial  [ "indexOfRefraction" ]->setFloat( 1.5f, 1.5f, 1.5f );
  sphereMaterial[ "indexOfRefraction" ]->setFloat( 1.5f, 1.5f, 1.5f );

  //
  // box
  //
  shapes_[ "box" ] = createShapeGroup(
                                      { boxPrim },
                                      { boxMaterial },
                                      "NoAccel",
                                      "NoAccel",
                                      optix::   make_float3( -1.5f, 0.0f, 0.0f )
                                      );

  //
  // quad
  //
  shapes_[ "ground" ] = createShapeGroup(
                                         { quadPrim },
                                         { quadMaterial },
                                         "NoAccel",
                                         "NoAccel",
                                         optix::make_float3( 0.0f, -1.0f, 0.0f ),
                                         optix::make_float3( 5.0f, 5.0f,  1.0f ),
                                         M_PIf * 0.5f,
                                         optix::make_float3( 1.0f, 0.0f,  0.0f )
                                         );


  //
  // sphere
  //
  shapes_[ "sphere" ] = createShapeGroup(
                                         { spherePrim },
                                         { sphereMaterial },
                                         "NoAccel",
                                         "NoAccel",
                                         optix::make_float3( 1.5f, 0.0f, 0.0f )
                                         );

  //
  // lights
  //
  Illuminator illuminator;
  illuminator.center      = optix::make_float3( 2.0f, 6.0f, 4.0f );
  illuminator.radiantFlux = optix::make_float3( 1000.0f );
  illuminator.shape       = LightShape::SPHERE;
  illuminator.radius      = 0.1f;

  shapes_[ "light" ] = createSphereIlluminator( illuminator, spherePrim );


  //
  // top group everything will get attached to
  //
  optix::Group topGroup = context_->createGroup( );
  topGroup->setChildCount( static_cast< unsigned >( shapes_.size( ) ) );

  unsigned index = 0;

  for ( auto & shapePair : shapes_ )
  {

    ShapeGroup &s = shapePair.second;
    attachToGroup( topGroup, s.group, index, s.transform );
    ++index;

  }

  topGroup->setAcceleration( context_->createAcceleration( "Bvh", "Bvh" ) );

  context_[ "top_object"   ]->set( topGroup );
  context_[ "top_shadower" ]->set( topGroup );

  context_[ "illuminators" ]->set( createInputBuffer( illuminators_ ) );

} // OptixBasicScene::_buildGeometry



} // namespace light
