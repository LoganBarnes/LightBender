#include "OptixAdvancedScene.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "imgui.h"



namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::OptixAdvancedScene
///////////////////////////////////////////////////////////////
OptixAdvancedScene::OptixAdvancedScene(
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
/// \brief OptixAdvancedScene::~OptixAdvancedScene
///////////////////////////////////////////////////////////////
OptixAdvancedScene::~OptixAdvancedScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::_buildScene
///////////////////////////////////////////////////////////////
void
OptixAdvancedScene::_buildScene( )
{

  optix::float3 albedo = optix::make_float3( 0.71f, 0.62f, 0.53f );   // clay
  float roughness      = 0.3f;

  // Create primitives used in the scene
  optix::Geometry boxPrim    = createBoxPrimitive( );
  optix::Geometry quadPrim   = createQuadPrimitive( );
  optix::Geometry spherePrim = createSpherePrimitive( );

  // Create materials used in the scene
  optix::Material groundMaterial = createMaterial(
                                                  materialPrograms_[ "closest_hit_bsdf" ],
                                                  materialPrograms_[ "any_hit_occlusion" ]
                                                  );
  optix::Material bigBoxMaterial = createMaterial(
                                                  materialPrograms_[ "closest_hit_bsdf" ],
                                                  materialPrograms_[ "any_hit_occlusion" ]
                                                  );
  optix::Material littleBoxMaterial = createMaterial(
                                                     materialPrograms_[ "closest_hit_bsdf" ],
                                                     materialPrograms_[ "any_hit_occlusion" ]
                                                     );
  optix::Material bigSphereMaterial = createMaterial(
                                                     materialPrograms_[ "closest_hit_bsdf" ],
                                                     materialPrograms_[ "any_hit_occlusion" ]
                                                     );
  optix::Material littleSphereMaterial = createMaterial(
                                                        materialPrograms_[ "closest_hit_bsdf" ],
                                                        materialPrograms_[ "any_hit_occlusion" ]
                                                        );

  groundMaterial      [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  bigBoxMaterial      [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  littleBoxMaterial   [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  bigSphereMaterial   [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  littleSphereMaterial[ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );

  groundMaterial      [ "roughness" ]->setFloat( roughness );
  bigBoxMaterial      [ "roughness" ]->setFloat( roughness );
  littleBoxMaterial   [ "roughness" ]->setFloat( roughness );
  bigSphereMaterial   [ "roughness" ]->setFloat( roughness );
  littleSphereMaterial[ "roughness" ]->setFloat( roughness );


  //
  // ground quad
  //
  shapes_[ "ground" ] = createShapeGroup(
                                         { quadPrim },
                                         { groundMaterial },
                                         "NoAccel",
                                         "NoAccel",
                                         optix::       make_float3( 0.0f ),
                                         optix::       make_float3( 5.0f, 5.0f, 1.0f ),
                                         M_PIf * 0.5f,
                                         optix::       make_float3( 1.0f, 0.0f, 0.0f )
                                         );

  //
  // stack of two boxes
  //
  shapes_[ "big box" ] = createShapeGroup(
                                          { boxPrim },
                                          { bigBoxMaterial },
                                          "NoAccel",
                                          "NoAccel",
                                          optix::      make_float3( -2.0f, 1.0f, -1.0f )
                                          );

  shapes_[ "little box" ] = createShapeGroup(
                                             { boxPrim },
                                             { littleBoxMaterial },
                                             "NoAccel",
                                             "NoAccel",
                                             optix::   make_float3(    -2.0f, 2.5f, -1.0f ),
                                             optix::   make_float3( 0.5f )
                                             );

  //
  // two spheres
  //
  shapes_[ "big sphere" ] = createShapeGroup(
                                             { spherePrim },
                                             { bigSphereMaterial },
                                             "NoAccel",
                                             "NoAccel",
                                             optix::   make_float3( 1.5f, 1.0f, 0.0f )
                                             );

  shapes_[ "little sphere" ] = createShapeGroup(
                                                { spherePrim },
                                                { littleSphereMaterial },
                                                "NoAccel",
                                                "NoAccel",
                                                optix::make_float3( 2.5f, 0.5f, 1.0f ),
                                                optix::make_float3( 0.5f )
                                                );

  //
  // lights
  //
  Illuminator illuminator;
  illuminator.center      = optix::make_float3( 1.0f, 4.0f, -3.0f );
  illuminator.radiantFlux = optix::make_float3( 2500.f );
  illuminator.shape       = LightShape::SPHERE;
  illuminator.radius      = 0.7f;

  shapes_[ "high light" ] = createSphereIlluminator( illuminator, spherePrim );


  illuminator.center      = optix::make_float3( -1.5f, 1.0f, 4.0f );
  illuminator.radiantFlux = optix::make_float3( 500.f );
  illuminator.shape       = LightShape::SPHERE;
  illuminator.radius      = 0.75f;

  shapes_[ "low light" ] = createSphereIlluminator( illuminator, spherePrim );



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


} // OptixAdvancedScene::_buildScene



} // namespace light
