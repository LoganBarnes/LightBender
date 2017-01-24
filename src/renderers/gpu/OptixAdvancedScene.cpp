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
  float roughness      = 0.2f;

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
  optix::Material wallMaterial = createMaterial(
                                                materialPrograms_[ "closest_hit_bsdf" ],
                                                materialPrograms_[ "any_hit_occlusion" ]
                                                );
  optix::Material redWallMaterial = createMaterial(
                                                     materialPrograms_[ "closest_hit_bsdf" ],
                                                     materialPrograms_[ "any_hit_occlusion" ]
                                                     );
  optix::Material greenWallMaterial = createMaterial(
                                                     materialPrograms_[ "closest_hit_bsdf" ],
                                                     materialPrograms_[ "any_hit_occlusion" ]
                                                     );

  groundMaterial      [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  bigBoxMaterial      [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  littleBoxMaterial   [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  bigSphereMaterial   [ "albedo" ]->setFloat( 0.8f, 0.3f, 0.7f );
  littleSphereMaterial[ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  wallMaterial        [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  redWallMaterial     [ "albedo" ]->setFloat( 0.8f, 0.2f, 0.3f );
  greenWallMaterial   [ "albedo" ]->setFloat( 0.2f, 0.8f, 0.3f );

  groundMaterial      [ "roughness" ]->setFloat( roughness );
  bigBoxMaterial      [ "roughness" ]->setFloat( 0.001f );
  littleBoxMaterial   [ "roughness" ]->setFloat( roughness );
  bigSphereMaterial   [ "roughness" ]->setFloat( roughness );
  littleSphereMaterial[ "roughness" ]->setFloat( 0.01f );
  wallMaterial        [ "roughness" ]->setFloat( roughness );
  redWallMaterial     [ "roughness" ]->setFloat( roughness );
  greenWallMaterial   [ "roughness" ]->setFloat( roughness );

  groundMaterial      [ "ior" ]->setFloat( 2.5f, 2.5f, 2.5f );
  bigBoxMaterial      [ "ior" ]->setFloat( 100.5f, 100.5f, 100.5f );
  littleBoxMaterial   [ "ior" ]->setFloat( 1.5f, 1.5f, 1.5f );
  bigSphereMaterial   [ "ior" ]->setFloat( 1.5f, 1.5f, 1.5f );
  littleSphereMaterial[ "ior" ]->setFloat( 10.0f, 10.0f, 10.0f );
  wallMaterial        [ "ior" ]->setFloat( 1.5f, 1.5f, 1.5f );
  redWallMaterial     [ "ior" ]->setFloat( 1.0f, 1.0f, 1.0f );
  greenWallMaterial   [ "ior" ]->setFloat( 1.0f, 1.0f, 1.0f );


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
  // walls
  //
  shapes_[ "back wall" ] = createShapeGroup(
                                            { quadPrim },
                                            { wallMaterial },
                                            "NoAccel",
                                            "NoAccel",
                                            optix::make_float3( 0.0f, 4.0f, -5.0f ),
                                            optix::make_float3( 5.0f, 4.0f, 1.0f ),
                                            M_PIf,
                                            optix::make_float3( 0.0f, 1.0f, 0.0f )
                                            );

  shapes_[ "left wall" ] = createShapeGroup(
                                            { quadPrim },
                                            { redWallMaterial },
                                            "NoAccel",
                                            "NoAccel",
                                            optix::make_float3( -5.0f, 4.0f, 0.0f ),
                                            optix::make_float3( 5.0f, 4.0f, 1.0f ),
                                            M_PIf * -0.5f,
                                            optix::make_float3( 0.0f, 1.0f, 0.0f )
                                            );

  shapes_[ "front wall" ] = createShapeGroup(
                                            { quadPrim },
                                            { wallMaterial },
                                            "NoAccel",
                                            "NoAccel",
                                            optix::make_float3( 0.0f, 4.0f, 5.0f ),
                                            optix::make_float3( 5.0f, 4.0f, 1.0f )
                                            );

  shapes_[ "green wall" ] = createShapeGroup(
                                            { quadPrim },
                                            { greenWallMaterial },
                                            "NoAccel",
                                            "NoAccel",
                                            optix::make_float3( 5.0f, 4.0f, 0.0f ),
                                            optix::make_float3( 5.0f, 4.0f, 1.0f ),
                                            M_PIf * 0.5f,
                                            optix::make_float3( 0.0f, 1.0f, 0.0f )
                                            );

  //
  // lights
  //
  Illuminator illuminator;
  illuminator.center      = optix::make_float3( 1.0f, 4.0f, -3.0f );
  illuminator.radiantFlux = optix::make_float3( 500.f );
  illuminator.shape       = LightShape::SPHERE;
  illuminator.radius      = 0.7f;

  shapes_[ "high light" ] = createSphereIlluminator( illuminator, spherePrim );


  illuminator.center      = optix::make_float3( -1.5f, 1.0f, 4.0f );
  illuminator.radiantFlux = optix::make_float3( 300.f );
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
