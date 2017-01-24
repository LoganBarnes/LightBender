#include "OptixModelScene.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "commonStructs.h"
#include "OptiXMesh.h"
#include "imgui.h"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixModelScene::OptixModelScene
///////////////////////////////////////////////////////////////
OptixModelScene::OptixModelScene(
                                 int               width,
                                 int               height,
                                 unsigned          vbo,
                                 const std::string &filename
                                 )
  : OptixScene( width, height, vbo )
{

  _buildScene( filename );

  context_->validate( );
  context_->compile( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixModelScene::~OptixModelScene
///////////////////////////////////////////////////////////////
OptixModelScene::~OptixModelScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixModelScene::_buildScene
///////////////////////////////////////////////////////////////
void
OptixModelScene::_buildScene( const std::string &filename )
{

  optix::float3 albedo = optix::make_float3( 0.71f, 0.62f, 0.53f );   // clay
  float roughness      = 0.3f;

  // Create primitives used in the scene
  optix::Geometry quadPrim   = createQuadPrimitive( );
  optix::Geometry spherePrim = createSpherePrimitive( );

  // Create materials used in the scene
  optix::Material groundMaterial = createMaterial(
                                                  materialPrograms_[ "closest_hit_bsdf" ],
                                                  materialPrograms_[ "any_hit_occlusion" ]
                                                  );

  optix::Material wallMaterial = createMaterial(
                                                materialPrograms_[ "closest_hit_bsdf" ],
                                                materialPrograms_[ "any_hit_occlusion" ]
                                                );

  optix::Material modelMaterial = createMaterial(
                                                 materialPrograms_[ "closest_hit_bsdf" ],
                                                 materialPrograms_[ "any_hit_occlusion" ]
                                                 );

  groundMaterial[ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  wallMaterial  [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  modelMaterial [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );

  groundMaterial[ "roughness" ]->setFloat( roughness );
  wallMaterial  [ "roughness" ]->setFloat( roughness );
  modelMaterial [ "roughness" ]->setFloat( roughness );

  groundMaterial[ "ior" ]->setFloat( 1.5f, 1.5f, 1.5f );
  wallMaterial  [ "ior" ]->setFloat( 1.5f, 1.5f, 1.5f );
  modelMaterial [ "ior" ]->setFloat( 1.5f, 1.5f, 1.5f );


  //
  // ground quad
  //
  shapes_[ "ground" ] = createShapeGroup(
                                         { quadPrim },
                                         { groundMaterial },
                                         "NoAccel",
                                         "NoAccel",
                                         optix::make_float3( 0.0f, -0.0f, 0.0f ),
                                         optix::make_float3( 8.0f, 8.0f, 1.0f ),
                                         M_PIf * 0.5f,
                                         optix::make_float3( 1.0f, 0.0f, 0.0f )
                                         );

  //
  // back wall
  //
  shapes_[ "back wall" ] = createShapeGroup(
                                            { quadPrim },
                                            { wallMaterial },
                                            "NoAccel",
                                            "NoAccel",
                                            optix::make_float3( 0.0f, 4.0f, -8.0f ),
                                            optix::make_float3( 8.0f, 4.0f, 1.0f ),
                                            M_PIf,
                                            optix::make_float3( 0.0f, 1.0f, 0.0f )
                                            );

  //
  // left wall
  //
  shapes_[ "left wall" ] = createShapeGroup(
                                            { quadPrim },
                                            { wallMaterial },
                                            "NoAccel",
                                            "NoAccel",
                                            optix::make_float3( -8.0f, 4.0f, 0.0f ),
                                            optix::make_float3( 8.0f, 4.0f, 1.0f ),
                                            M_PIf * -0.5f,
                                            optix::make_float3( 0.0f, 1.0f, 0.0f )
                                            );

  //
  // mesh geom group
  //
  OptiXMesh mesh;
  mesh.context  = context_;
  mesh.material = modelMaterial;

  loadMesh( filename, mesh, light::RES_PATH + "ptx/" );

  shapes_[ "model" ] = createShapeGroup(
                                        { mesh.geom_instance },
                                        "Trbvh",
                                        "Bvh",
                                        optix::make_float3( 0.0f, 3.0f, 0.0f ),
                                        optix::make_float3( 0.01f )
                                        );


  shapes_[ "model" ].materials.push_back( mesh.material );


  //
  // lights 1.362f W/m^2 at surface
  //
  Illuminator illuminator;
  illuminator.center      = optix::normalize( optix::make_float3( 4.0f, 10.0f, 4.0f ) ) * 149.6e9f;
  illuminator.radiantFlux = optix::make_float3( 4.1e26f ) * 0.002f; // 0.002f for arbitrary atmosphere
  illuminator.shape       = LightShape::SPHERE;
  illuminator.radius      = 695.7e6f;

  shapes_[ "light1" ] = createSphereIlluminator( illuminator, spherePrim );


  //
  // top group everything will get attached to
  //
  optix::Group topGroup = context_->createGroup( );
  topGroup->setChildCount( static_cast< unsigned >( shapes_.size( ) ) );

  unsigned index = 0;

  for ( auto &shapePair : shapes_ )
  {

    ShapeGroup &s = shapePair.second;
    attachToGroup( topGroup, s.group, index, s.transform );
    ++index;

  }

  topGroup->setAcceleration( context_->createAcceleration( "Bvh", "Bvh" ) );

  context_[ "top_object"   ]->set( topGroup );
  context_[ "top_shadower" ]->set( topGroup );

  context_[ "illuminators" ]->set( createInputBuffer( illuminators_ ) );

} // OptixModelScene::_buildScene





} // namespace light
