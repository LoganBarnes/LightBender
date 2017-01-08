#include "OptixModelScene.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "commonStructs.h"
#include "OptiXMesh.h"


namespace light
{

const optix::float3 lightLocation = optix::make_float3( 2.0f, 6.0f, 4.0f );
const optix::float3 lightPower    = optix::make_float3( 120.f );
constexpr float lightRadius       = 0.1f;


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

  _buildGeometry( filename );
  _addLights( );

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
OptixModelScene::_buildGeometry( const std::string &filename )
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
  optix::Geometry spherePrim = createSpherePrimitive( );

  // top group everything will get attached to
  optix::Group topGroup = context_->createGroup( );
  topGroup->setChildCount( 2 );

  // mesh geom group
  OptiXMesh mesh;

  mesh.context = context_;
  loadMesh( filename, mesh );

  optix::GeometryGroup meshGroup = context_->createGeometryGroup( );
  meshGroup->addChild( mesh.geom_instance );
  meshGroup->setAcceleration( context_->createAcceleration( "Trbvh" ) );


  // lights
  optix::GeometryGroup lightSphereGroup = createGeomGroup(
                                                          { spherePrim },
                                                          { lightMaterial },
                                                          "NoAccel",
                                                          "NoAccel"
                                                          );

  // ground quad
  attachToGroup(
                topGroup,
                meshGroup,
                0,
                optix::make_float3( 0.0f, 0.0f, 5.0f )
                );

  // light
  attachToGroup(
                topGroup, lightSphereGroup, 1,
                lightLocation,
                optix::make_float3( lightRadius )
                );


  topGroup->setAcceleration( context_->createAcceleration( "Bvh", "Bvh" ) );

  context_[ "top_object"   ]->set( topGroup );
  context_[ "top_shadower" ]->set( topGroup );

} // OptixModelScene::_buildScene



///////////////////////////////////////////////////////////////
/// \brief OptixModelScene::_addLights
///////////////////////////////////////////////////////////////
void
OptixModelScene::_addLights( )
{

  std::vector< Light > lights =
  {

    createLight(
                lightLocation,
                lightPower,
                LightShape::SPHERE,
                lightRadius
                )

  };

  float area = M_PIf * 4.0f * lightRadius * lightRadius;

  context_[ "emissionRadiance" ]->setFloat(
                                           lightPower
                                           / ( M_PIf * area )
                                           );

  context_[ "lights" ]->set( createInputBuffer( lights ) );

} // OptixModelScene::_addLights



} // namespace light
