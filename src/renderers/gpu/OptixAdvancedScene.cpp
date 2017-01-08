#include "OptixAdvancedScene.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "commonStructs.h"
#include "imgui.h"


#define BIG_AND_CLOSE


namespace light
{

#ifndef BIG_AND_CLOSE

const optix::float3 lightLocation = optix::make_float3( 2.0f, 6.0f, 4.0f );
const optix::float3 lightPower    = optix::make_float3( 120.f );
constexpr float lightRadius       = 0.1f;

#else

const optix::float3 lightLocation = optix::make_float3( 1.0f, 4.0f, -3.0f );
const optix::float3 lightPower    = optix::make_float3( 2500.f );
constexpr float lightRadius       = 0.7f;

#endif


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

  _buildGeometry( );
  _addLights( );

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
OptixAdvancedScene::_buildGeometry( )
{

  optix::Material lightMaterial = context_->createMaterial( );

  // Materials
  std::string brdfPtxFile(
                          light::RES_PATH
                          + "ptx/cudaLightBender_generated_Brdf.cu.ptx"
                          );

  lightMaterial->setClosestHitProgram( 0, context_->createProgramFromPTXFile(
                                                                             brdfPtxFile,
                                                                             "closest_hit_emission"
                                                                             ) );


  // Create primitives used in the scene
  optix::Geometry quadPrim   = createQuadPrimitive( );
  optix::Geometry boxPrim    = createBoxPrimitive( );
  optix::Geometry spherePrim = createSpherePrimitive( );

  // top group everything will get attached to
  optix::Group topGroup = context_->createGroup( );
  topGroup->setChildCount(
      #ifdef BIG_AND_CLOSE
        7
      #else
        6
      #endif
        );

  // attach materials to geometries
  optix::GeometryGroup quadGroup = createGeomGroup(
                                                   { quadPrim },
                                                   { sceneMaterial_ },
                                                   "NoAccel",
                                                   "NoAccel"
                                                   );

  optix::GeometryGroup boxGroup = createGeomGroup(
                                                  { boxPrim },
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

  optix::GeometryGroup lightSphereGroup = createGeomGroup(
                                                          { spherePrim },
                                                          { lightMaterial },
                                                          "NoAccel",
                                                          "NoAccel"
                                                          );


  // ground quad
  attachToGroup(
                topGroup,
                quadGroup,
                0,
                optix::make_float3( 0.0f ),
                optix::make_float3( 5.0f, 5.0f, 1.0f ),
                M_PIf * 0.5f,
                optix::make_float3( 1.0f, 0.0f, 0.0f )
                );

  // stack of two boxes
  attachToGroup( topGroup, boxGroup, 1, optix::make_float3( -2.0f, 1.0f, -1.0f ) );
  attachToGroup(
                topGroup, boxGroup, 2,
                optix::make_float3(    -2.0f, 2.5f, -1.0f ),
                optix::make_float3( 0.5f )
                );

  // two spheres
  attachToGroup( topGroup, sphereGroup, 3, optix::make_float3( 1.5f, 1.0f, 0.0f ) );
  attachToGroup(
                topGroup, sphereGroup, 4,
                optix::make_float3( 2.5f, 0.5f, 1.0f ),
                optix::make_float3( 0.5f )
                );

  // light
  attachToGroup(
                topGroup, lightSphereGroup, 5,
                lightLocation,
                optix::make_float3( lightRadius )
                );


#ifdef BIG_AND_CLOSE

  // light
  attachToGroup(
                topGroup, lightSphereGroup, 6,
                optix::make_float3( -1.5f, 1.0f, 4.0f ),
                optix::make_float3( 0.75f )
                );
#endif


  topGroup->setAcceleration( context_->createAcceleration( "Bvh", "Bvh" ) );

  context_[ "top_object"   ]->set( topGroup );
  context_[ "top_shadower" ]->set( topGroup );

} // OptixAdvancedScene::_buildScene



///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::_addLights
///////////////////////////////////////////////////////////////
void
OptixAdvancedScene::_addLights( )
{

  std::vector< Light > lights =
  {

    createLight(
                lightLocation,
                lightPower,
                LightShape::SPHERE,
                lightRadius
                )

  #ifdef BIG_AND_CLOSE

    , createLight(
                  optix::float3 { -1.5f, 1.0f, 4.0f },
                  optix::float3 { 500.0f, 500.0f, 500.0f },
                  LightShape::SPHERE,
                  0.75f
                  )
  #endif

  };

  float area = M_PIf * 4.0f * lightRadius * lightRadius;

  context_[ "emissionRadiance" ]->setFloat(
                                           lightPower
                                           / ( M_PIf * area )
                                           );

  context_[ "lights" ]->set( createInputBuffer( lights ) );

} // OptixAdvancedScene::_addLights



///////////////////////////////////////////////////////////////
/// \brief OptixAdvancedScene::renderSceneGui
///
///        Allows for specific manipulation of each scene
///////////////////////////////////////////////////////////////
void
OptixAdvancedScene::renderSceneGui()
{

  if ( ImGui::CollapsingHeader( "Advanced Scene", "modelScene", false, true ) )
  {

    ImGui::Text( "Blah" );

  }

}


} // namespace light
