#include "OptixBasicScene.hpp"
#include <sstream>
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "commonStructs.h"
#include "imgui.h"


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

  float3 albedo   = optix::make_float3( 0.71f, 0.62f, 0.53f ); // clay
  float roughness = 0.3f;

  // Create primitives used in the scene
  optix::Geometry boxPrim    = createBoxPrimitive( );
  optix::Geometry quadPrim   = createQuadPrimitive( );
  optix::Geometry spherePrim = createSpherePrimitive( );

  // Create materials used in the scene
  optix::Material boxMaterial    = createMaterial( materialPrograms_[ 2 ], materialPrograms_[ 4 ] );
  optix::Material quadMaterial   = createMaterial( materialPrograms_[ 2 ], materialPrograms_[ 4 ] );
  optix::Material sphereMaterial = createMaterial( materialPrograms_[ 2 ], materialPrograms_[ 4 ] );
  optix::Material lightMaterial  = createMaterial( materialPrograms_[ 3 ], nullptr );

  boxMaterial   [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  quadMaterial  [ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );
  sphereMaterial[ "albedo" ]->setFloat( albedo.x, albedo.y, albedo.z );

  boxMaterial   [ "roughness" ]->setFloat( roughness );
  quadMaterial  [ "roughness" ]->setFloat( roughness );
  sphereMaterial[ "roughness" ]->setFloat( roughness );

  //
  // box
  //
  shapes_.push_back ( createShapeGroup(
                                       { boxPrim },
                                       { boxMaterial },
                                       "NoAccel",
                                       "NoAccel",
                                       optix::make_float3( -1.5f, 0.0f, 0.0f )
                                       ) );

  //
  // quad
  //
  shapes_.push_back ( createShapeGroup(
                                       { quadPrim },
                                       { quadMaterial },
                                       "NoAccel",
                                       "NoAccel",
                                       optix::make_float3( 0.0f,    -1.0f, 0.0f ),
                                       optix::make_float3( 5.0f, 5.0f,     1.0f ),
                                       M_PIf * 0.5f,
                                       optix::make_float3( 1.0f, 0.0f,     0.0f )
                                       ) );


  //
  // sphere
  //
  shapes_.push_back ( createShapeGroup(
                                       { spherePrim },
                                       { sphereMaterial },
                                       "NoAccel",
                                       "NoAccel",
                                       optix::make_float3( 1.5f, 0.0f, 0.0f )
                                       ) );

  //
  // light
  //
  shapes_.push_back( createShapeGroup(
                                      { spherePrim },
                                      { lightMaterial },
                                      "NoAccel",
                                      "NoAccel",
                                      optix::make_float3( 2.0f, 6.0f, 4.0f ),
                                      optix::make_float3( 0.1f )
                                      ) );


  // top group everything will get attached to
  optix::Group topGroup = context_->createGroup( );
  topGroup->setChildCount( shapes_.size( ) );

  for ( unsigned i = 0; i < shapes_.size( ); ++i )
  {

    ShapeGroup &s = shapes_[ i ];
    attachToGroup( topGroup, s.group, i, s.transform );

  }

  topGroup->setAcceleration( context_->createAcceleration( "Bvh", "Bvh" ) );

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
                optix::make_float3( 2.0f, 6.0f, 4.0f ),
                optix::make_float3( 120.0f ),
                LightShape::SPHERE,
                0.1f
                )

  };

  float area      = M_PIf * 4.0f * 0.1f * 0.1f;
  float3 emission = optix::make_float3( 120.0f ) / ( M_PIf * area );

  shapes_.back( ).materials[ 0 ][ "emissionRadiance" ]->setFloat( emission );

  context_[ "lights" ]->set( createInputBuffer( lights ) );

} // OptixBasicScene::_addLights



///////////////////////////////////////////////////////////////
/// \brief OptixBasicScene::renderSceneGui
///
///        Allows for specific manipulation of each scene
///////////////////////////////////////////////////////////////
void
OptixBasicScene::renderSceneGui( )
{

  if ( ImGui::CollapsingHeader( "Basic Scene", "basicScene", false, true ) )
  {

    std::stringstream stream;

    for ( unsigned i = 0; i < shapes_.size( ) - 1; ++i )
    {


      ImGui::Separator( );

      ImGui::Text( "Shape %d", i );

      ShapeGroup &s = shapes_[ i ];

      //
      // albedo
      //
      float3 albedo        = s.materials[ 0 ][ "albedo" ]->getFloat3( );
      float albedoOld[ 3 ] = { albedo.x, albedo.y, albedo.z };
      float albedoNew[ 3 ] = { albedo.x, albedo.y, albedo.z };

      stream << "Albedo " << i;
      ImGui::ColorEdit3( stream.str( ).c_str( ), albedoNew );
      stream.str( std::string( ) );

      if (
          albedoOld[ 0 ] != albedoNew[ 0 ]
          || albedoOld[ 1 ] != albedoNew[ 1 ]
          || albedoOld[ 2 ] != albedoNew[ 2 ]
          )
      {

        s.materials[ 0 ][ "albedo" ]->setFloat( albedoNew[ 0 ], albedoNew[ 1 ], albedoNew[ 2 ] );
        resetFrameCount( );

      }

      //
      // roughness
      //
      float roughness    = s.materials[ 0 ][ "roughness" ]->getFloat( );
      float roughnessOld = roughness;

      stream << "Roughness " << i;
      ImGui::SliderFloat( stream.str( ).c_str( ), &roughness, 0.0, 1.0 );
      stream.str( std::string( ) );

      if ( roughnessOld != roughness )
      {

        s.materials[ 0 ][ "roughness" ]->setFloat( roughness );
        resetFrameCount( );

      }

    }

    ImGui::Separator( );

    ImGui::Text( "Light %d", 0 );

  }

} // OptixBasicScene::renderSceneGui



} // namespace light
