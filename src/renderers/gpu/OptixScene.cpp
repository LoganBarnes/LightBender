#include "OptixScene.hpp"
#include <sstream>
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "imgui.h"


namespace light
{



///////////////////////////////////////////////////////////////
/// \brief OptixScene::OptixScene
///////////////////////////////////////////////////////////////
OptixScene::OptixScene(
                       int      width,
                       int      height,
                       unsigned vbo
                       )
  : OptixRenderer ( width, height, vbo )
  , sceneMaterial_( context_->createMaterial( ) )
{

  //
  // Various ways to display objects
  //
  std::string brdfPtxFile(
                          light::RES_PATH
                          + "ptx/cudaLightBender_generated_Brdf.cu.ptx"
                          );

  std::string name = "closest_hit_normals";

  materialNames_.push_back( name );
  materialPrograms_[ name ] = context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 name
                                                                 );

  name = "closest_hit_simple_shading";
  materialNames_.push_back( name );
  materialPrograms_[ name ] = context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 name
                                                                 );

  name = "closest_hit_bsdf";
  materialNames_.push_back( name );
  materialPrograms_[ name ] = context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 name
                                                                 );



  //
  // for lights
  //
  name = "closest_hit_emission";
  materialNames_.push_back( name );
  materialPrograms_[ name ] = context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 name
                                                                 );


  //
  // for shadowing
  //
  name = "any_hit_occlusion";
  materialNames_.push_back( name );
  materialPrograms_[ name ] = context_->createProgramFromPTXFile(
                                                                 brdfPtxFile,
                                                                 name
                                                                 );

  sceneMaterial_->setClosestHitProgram( 0, materialPrograms_[ "closest_hit_bsdf" ] );
  sceneMaterial_->setAnyHitProgram( 1, materialPrograms_[ "any_hit_occlusion" ] );


  // defaults
  setDisplayType( 2 ); // oren-nayar diffuse brdf
  setCameraType ( 0 ); // basic non-pathtracer
  setSqrtSamples( 1 ); // single sample per pixel on each pass
  setMaxBounces ( 5 ); // only allow 5 bounces
  setFirstBounce( 0 ); // start rendering on first bounce

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::~OptixScene
///////////////////////////////////////////////////////////////
OptixScene::~OptixScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::setDisplayType
/// \param type
///////////////////////////////////////////////////////////////
void
OptixScene::setDisplayType( int type )
{

  std::string programName       = materialNames_[ static_cast< size_t >( type ) ];
  optix::Program currentProgram = materialPrograms_[ programName ];

  sceneMaterial_->setClosestHitProgram( 0, currentProgram );

  for ( auto &shapePair : shapes_ )
  {

    ShapeGroup &s = shapePair.second;

    if ( s.illuminatorIndex < 0 )
    {

      for ( optix::Material &mat : s.materials )
      {

        mat->setClosestHitProgram( 0, currentProgram );

      }

    }

  }

  resetFrameCount( );

} // OptixScene::setDisplayType



///////////////////////////////////////////////////////////////
/// \brief OptixScene::setMaxBounces
/// \param bounces
///////////////////////////////////////////////////////////////
void
OptixScene::setMaxBounces( unsigned bounces )
{

  context_[ "max_bounces" ]->setUint( bounces );

  resetFrameCount( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::setFirstBounce
/// \param bounce
///////////////////////////////////////////////////////////////
void
OptixScene::setFirstBounce( unsigned bounce )
{

  context_[ "first_bounce" ]->setUint( bounce );

  resetFrameCount( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createBoxPrimitive
/// \param min
/// \param max
/// \return
///////////////////////////////////////////////////////////////
optix::Geometry
OptixScene::createBoxPrimitive(
                               optix::float3 min,
                               optix::float3 max
                               )
{

  // Create box
  std::string box_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Box.cu.ptx" );
  optix::Program box_bounds    = context_->createProgramFromPTXFile( box_ptx, "box_bounds" );
  optix::Program box_intersect = context_->createProgramFromPTXFile( box_ptx, "box_intersect" );

  optix::Geometry box = context_->createGeometry( );

  box->setPrimitiveCount( 1u );
  box->setBoundingBoxProgram( box_bounds );
  box->setIntersectionProgram( box_intersect );
  box[ "boxmin" ]->setFloat( min );
  box[ "boxmax" ]->setFloat( max );

  return box;

} // OptixScene::createBoxPrimitive



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createSpherePrimitive
/// \param center
/// \param radius
/// \return
///////////////////////////////////////////////////////////////
optix::Geometry
OptixScene::createSpherePrimitive(
                                  optix::float3 center,
                                  float         radius
                                  )
{

  // Create sphere
  std::string sphere_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Sphere.cu.ptx" );
  optix::Program sphere_bounds    = context_->createProgramFromPTXFile( sphere_ptx, "bounds" );
  optix::Program sphere_intersect = context_->createProgramFromPTXFile( sphere_ptx, "intersect" );

  optix::Geometry sphere = context_->createGeometry( );

  sphere->setPrimitiveCount( 1u );
  sphere->setBoundingBoxProgram ( sphere_bounds );
  sphere->setIntersectionProgram( sphere_intersect );
  sphere[ "sphere" ]->setFloat( center.x, center.y, center.z, radius );

  return sphere;

} // createSpherePrimitive



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createQuadPrimitive
/// \param anchor
/// \param v1
/// \param v2
/// \return
///////////////////////////////////////////////////////////////
optix::Geometry
OptixScene::createQuadPrimitive(
                                optix::float3 anchor,
                                optix::float3 v1,
                                optix::float3 v2
                                )
{

  // Create quad
  std::string quad_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Parallelogram.cu.ptx" );
  optix::Program quad_bounds    = context_->createProgramFromPTXFile( quad_ptx, "bounds" );
  optix::Program quad_intersect = context_->createProgramFromPTXFile( quad_ptx, "intersect" );

  optix::Geometry quad = context_->createGeometry( );

  quad->setPrimitiveCount( 1u );
  quad->setBoundingBoxProgram( quad_bounds );
  quad->setIntersectionProgram( quad_intersect );

  optix::float3 normal = optix::cross( v1, v2 );

  normal = normalize( normal );

  float d = dot( normal, anchor );
  v1 *= 1.0f / dot( v1, v1 );
  v2 *= 1.0f / dot( v2, v2 );
  optix::float4 plane = optix::make_float4( normal, d );

  quad[ "plane"  ]->setFloat( plane.x, plane.y, plane.z, plane.w );
  quad[ "v1"     ]->setFloat( v1.x, v1.y, v1.z );
  quad[ "v2"     ]->setFloat( v2.x, v2.y, v2.z );
  quad[ "anchor" ]->setFloat( anchor.x, anchor.y, anchor.z );

  return quad;

} // OptixScene::createQuadPrimitive



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createMaterial
/// \param closestHitProgram
/// \param anyHitProgram
/// \return
///////////////////////////////////////////////////////////////
optix::Material
OptixScene::createMaterial(
                           optix::Program closestHitProgram,
                           optix::Program anyHitProgram
                           )
{

  optix::Material material = context_->createMaterial( );

  material->setClosestHitProgram( 0, closestHitProgram );

  if ( anyHitProgram )
  {

    material->setAnyHitProgram ( 1, anyHitProgram );

  }

  return material;

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createGeomGroup
/// \param geometries
/// \param materials
/// \param builderAccel
/// \param traverserAccel
/// \return
///////////////////////////////////////////////////////////////
optix::GeometryGroup
OptixScene::createGeomGroup(
                            const std::vector< optix::Geometry > &geometries,
                            const std::vector< optix::Material > &materials,
                            const std::string                    &builderAccel,
                            const std::string                    &traverserAccel
                            )
{

  // check for one to one mapping of geometries and materials
  if ( materials.size( ) != geometries.size( ) )
  {

    throw std::runtime_error( "Geometries and Materials must contain the same number of elements" );

  }

  unsigned numInstances = static_cast< unsigned >( geometries.size( ) );

  // fill out geometry group for all geometries and materials
  optix::GeometryGroup geometryGroup = context_->createGeometryGroup( );
  geometryGroup->setChildCount( numInstances );

  for ( unsigned i = 0; i < numInstances; ++i )
  {

    optix::GeometryInstance gi = context_->createGeometryInstance( );

    gi->setGeometry( geometries[ i ] );
    gi->setMaterialCount( 1 );
    gi->setMaterial( 0, materials[ i ] );

    geometryGroup->setChild( i, gi );

  }

  geometryGroup->setAcceleration( context_->createAcceleration(
                                                               builderAccel.c_str( ),
                                                               traverserAccel.c_str( )
                                                               ) );

  return geometryGroup;

} // OptixScene::createGeomGroup



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createShapeGeomGroup
/// \param pShape
///////////////////////////////////////////////////////////////
void
OptixScene::createShapeGeomGroup( ShapeGroup *pShape )
{

  pShape->group = createGeomGroup(
                                  pShape->geometries,
                                  pShape->materials,
                                  pShape->builderAccel,
                                  pShape->traverserAccel
                                  );

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::attachToGroup
/// \param group
/// \param geomGroup
/// \param translation
/// \param scale
/// \param rotationAngle
/// \param rotationAxis
///////////////////////////////////////////////////////////////
void
OptixScene::attachToGroup(
                          optix::Group         group,
                          optix::GeometryGroup geomGroup,
                          unsigned             childNum,
                          optix::float3        translation,
                          optix::float3        scale,
                          float                rotationAngle,
                          optix::float3        rotationAxis
                          )
{

  optix::Transform trans = context_->createTransform( );

  trans->setChild( geomGroup );
  group->setChild( childNum, trans );

  optix::Matrix4x4 T = optix::Matrix4x4::translate( translation );
  optix::Matrix4x4 S = optix::Matrix4x4::scale( scale );
  optix::Matrix4x4 R = optix::Matrix4x4::rotate( rotationAngle, rotationAxis );

  optix::Matrix4x4 M = T * R * S;

  trans->setMatrix( false, M.getData( ), 0 );

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::attachToGroup
/// \param group
/// \param geomGroup
/// \param childNum
/// \param M
///////////////////////////////////////////////////////////////
void
OptixScene::attachToGroup(
                          optix::Group         group,
                          optix::GeometryGroup geomGroup,
                          unsigned             childNum,
                          optix::Matrix4x4     M
                          )
{

  optix::Transform trans = context_->createTransform( );

  trans->setChild( geomGroup );
  group->setChild( childNum, trans );

  trans->setMatrix( false, M.getData( ), 0 );

}



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createShapeGroup
/// \param geometries
/// \param materials
/// \param builderAccel
/// \param traverserAccel
/// \param translation
/// \param scale
/// \param rotationAngle
/// \param rotationAxis
/// \return
///////////////////////////////////////////////////////////////
ShapeGroup
OptixScene::createShapeGroup(
                             const std::vector< optix::Geometry > &geometries,
                             const std::vector< optix::Material > &materials,
                             const std::string                    &builderAccel,
                             const std::string                    &traverserAccel,
                             optix::float3                        translation,
                             optix::float3                        scale,
                             float                                rotationAngle,
                             optix::float3                        rotationAxis
                             )
{

  ShapeGroup shape;

  shape.geometries       = geometries;
  shape.materials        = materials;
  shape.builderAccel     = builderAccel;
  shape.traverserAccel   = traverserAccel;
  shape.illuminatorIndex = -1;

  optix::Matrix4x4 T = optix::Matrix4x4::translate( translation );
  optix::Matrix4x4 S = optix::Matrix4x4::scale( scale );
  optix::Matrix4x4 R = optix::Matrix4x4::rotate( rotationAngle, rotationAxis );

  shape.transform = T * R * S;

  createShapeGeomGroup( &shape );

  return shape;

} // OptixScene::createShapeGroup



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createShapeGroup
/// \param geomInstances
/// \param builderAccel
/// \param traverserAccel
/// \param translation
/// \param scale
/// \param rotationAngle
/// \param rotationAxis
/// \return
///////////////////////////////////////////////////////////////
ShapeGroup
OptixScene::createShapeGroup(
                             const std::vector< optix::GeometryInstance > geomInstances,
                             const std::string                            &builderAccel,
                             const std::string                            &traverserAccel,
                             optix::float3                                translation,
                             optix::float3                                scale,
                             float                                        rotationAngle,
                             optix::float3                                rotationAxis
                             )
{

  ShapeGroup shape;

  shape.builderAccel     = builderAccel;
  shape.traverserAccel   = traverserAccel;
  shape.illuminatorIndex = -1;

  optix::Matrix4x4 T = optix::Matrix4x4::translate( translation );
  optix::Matrix4x4 S = optix::Matrix4x4::scale( scale );
  optix::Matrix4x4 R = optix::Matrix4x4::rotate( rotationAngle, rotationAxis );

  shape.transform = T * R * S;

  unsigned numInstances = static_cast< unsigned >( geomInstances.size( ) );

  // fill out geometry group for all geometries and materials
  shape.group = context_->createGeometryGroup( );
  shape.group->setChildCount( numInstances );

  for ( unsigned i = 0; i < numInstances; ++i )
  {

    shape.group->setChild( i, geomInstances[ i ] );

  }

  shape.group->setAcceleration( context_->createAcceleration(
                                                             builderAccel.c_str( ),
                                                             traverserAccel.c_str( )
                                                             ) );

  return shape;

} // OptixScene::createShapeGroup



///////////////////////////////////////////////////////////////
/// \brief OptixScene::createSphereIlluminator
/// \param illuminator
/// \param spherePrimitive
/// \return
///////////////////////////////////////////////////////////////
ShapeGroup
OptixScene::createSphereIlluminator(
                                    const Illuminator &illuminator,
                                    optix::Geometry   spherePrimitive
                                    )
{

  illuminators_.push_back( illuminator );

  optix::Material material = context_->createMaterial( );
  material->setClosestHitProgram( 0, materialPrograms_[ "closest_hit_emission" ] );

  float area             = M_PIf * 4.0f * illuminator.radius * illuminator.radius;
  optix::float3 emission = illuminator.radiantFlux / ( M_PIf * area );

  material[ "emissionRadiance" ]->setFloat( emission );

  ShapeGroup shape = createShapeGroup(
                                      { spherePrimitive },
                                      { material },
                                      "NoAccel",
                                      "NoAccel",
                                      illuminator.center,
                                      optix::make_float3( illuminator.radius )
                                      );

  // illuminators_.size() is at least 1 since we added one above
  shape.illuminatorIndex = static_cast< int >( illuminators_.size( ) ) - 1;

  return shape;

} // OptixScene::createSphereIlluminator



///////////////////////////////////////////////////////////////
/// \brief Optixcene::renderSceneGui
///
///        Allows for specific manipulation of each scene
///////////////////////////////////////////////////////////////
void
OptixScene::renderSceneGui( )
{

  if ( ImGui::CollapsingHeader( "Scene Settings", "defaultScene", false, true ) )
  {

    std::stringstream stream;

    //
    // Non-emitting shapes
    //
    for ( auto &shapePair : shapes_ )
    {

      ShapeGroup &shape       = shapePair.second;
      const std::string &name = shapePair.first;

      if ( shape.illuminatorIndex < 0 )
      {

        ImGui::Separator( );

        ImGui::Text( "%s", name.c_str( ) );


        //
        // albedo
        //
        optix::float3 albedo = shape.materials[ 0 ][ "albedo" ]->getFloat3( );
        float albedoOld[ 3 ] = { albedo.x, albedo.y, albedo.z };
        float albedoNew[ 3 ] = { albedo.x, albedo.y, albedo.z };

        stream << "Albedo " << name;
        ImGui::ColorEdit3( stream.str( ).c_str( ), albedoNew );
        stream.str( std::string( ) );

        if (
            albedoOld[ 0 ] != albedoNew[ 0 ]
            || albedoOld[ 1 ] != albedoNew[ 1 ]
            || albedoOld[ 2 ] != albedoNew[ 2 ]
            )
        {

          shape.materials[ 0 ][ "albedo" ]->setFloat(
                                                     albedoNew[ 0 ],
                                                     albedoNew[ 1 ],
                                                     albedoNew[ 2 ]
                                                     );
          resetFrameCount( );

        }


        //
        // roughness
        //
        float roughness    = shape.materials[ 0 ][ "roughness" ]->getFloat( );
        float roughnessOld = roughness;

        stream << "Roughness " << name;
        ImGui::SliderFloat( stream.str( ).c_str( ), &roughness, 0.0, 1.0 );
        stream.str( std::string( ) );

        if ( roughnessOld != roughness )
        {

          shape.materials[ 0 ][ "roughness" ]->setFloat( roughness );
          resetFrameCount( );

        }


        //
        // albedo
        //
        optix::float3 ior = shape.materials[ 0 ][ "ior" ]->getFloat3( );
        float iorOld[ 3 ] = { ior.x, ior.y, ior.z };
        float iorNew[ 3 ] = { ior.x, ior.y, ior.z };

        stream << "IOR " << name;
        ImGui::SliderFloat3( stream.str( ).c_str( ), iorNew, 1.0f, 10.0f );
        stream.str( std::string( ) );

        if (
            iorOld[ 0 ] != iorNew[ 0 ]
            || iorOld[ 1 ] != iorNew[ 1 ]
            || iorOld[ 2 ] != iorNew[ 2 ]
            )
        {

          shape.materials[ 0 ][ "ior" ]->setFloat(
                                                  iorNew[ 0 ],
                                                  iorNew[ 1 ],
                                                  iorNew[ 2 ]
                                                  );
          resetFrameCount( );

        }

      }

    } // shapes for loop



    //
    // Illuminators
    //
    bool updateLightBuffer = false;

    for ( auto &shapePair : shapes_ )
    {

      ShapeGroup &shape       = shapePair.second;
      const std::string &name = shapePair.first;

      if ( shape.illuminatorIndex >= 0 )
      {

        Illuminator &i = illuminators_[ static_cast< unsigned >( shape.illuminatorIndex ) ];

        ImGui::Separator( );

        ImGui::Text( "%s", name.c_str( ) );

        float fluxOld[ 3 ] = { i.radiantFlux.x, i.radiantFlux.y, i.radiantFlux.z };

        stream << "Power (W) " << name;
        ImGui::SliderFloat3( stream.str( ).c_str( ), fluxOld, 1.0f, 3000.0f );
        stream.str( std::string( ) );

        if ( fluxOld[ 0 ] != i.radiantFlux.x
            || fluxOld[ 1 ] != i.radiantFlux.y
            || fluxOld[ 2 ] != i.radiantFlux.z )
        {

          i.radiantFlux     = optix::make_float3( fluxOld[ 0 ], fluxOld[ 1 ], fluxOld[ 2 ] );
          updateLightBuffer = true;

        }

      }

    } // lights for loop

    if ( updateLightBuffer )
    {

      optix::Buffer illumBuf = context_[ "illuminators" ]->getBuffer( );
      illumBuf->destroy( );

      context_[ "illuminators" ]->setBuffer( createInputBuffer( illuminators_ ) );

      resetFrameCount( );

    }

  } // collapsing header

} // OptixBasicScene::renderSceneGui



} // namespace light
