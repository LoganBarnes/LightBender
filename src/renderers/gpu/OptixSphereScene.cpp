#include "OptixSphereScene.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixSphereScene::OptixSphereScene
///////////////////////////////////////////////////////////////
OptixSphereScene::OptixSphereScene(
                                   int      width,
                                   int      height,
                                   unsigned vbo
                                   )
  : OptixRenderer( width, height, vbo )

{

  _buildScene( );

  context_->validate( );
  context_->compile( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixSphereScene::~OptixSphereScene
///////////////////////////////////////////////////////////////
OptixSphereScene::~OptixSphereScene( )
{}



///////////////////////////////////////////////////////////////
/// \brief OptixSphereScene::_buildScene
///////////////////////////////////////////////////////////////
void
OptixSphereScene::_buildScene( )
{

  std::string box_ptx( light::RES_PATH + "ptx/cudaLightBender_generated_Box.cu.ptx" );
  optix::Program box_bounds    = context_->createProgramFromPTXFile( box_ptx, "box_bounds" );
  optix::Program box_intersect = context_->createProgramFromPTXFile( box_ptx, "box_intersect" );

  // Create box
  optix::Geometry box = context_->createGeometry( );
  box->setPrimitiveCount( 1u );
  box->setBoundingBoxProgram( box_bounds );
  box->setIntersectionProgram( box_intersect );
  box[ "boxmin" ]->setFloat( -4.0f, -1.5f, -4.0f );
  box[ "boxmax" ]->setFloat(  4.0f,  2.0f,  4.0f );

  // Materials
  std::string brdfPtxFile(
                          light::RES_PATH
                          + "ptx/cudaLightBender_generated_Brdf.cu.ptx"
                          );

  optix::Material box_matl = context_->createMaterial( );
  optix::Program box_ch    = context_->createProgramFromPTXFile(
                                                                brdfPtxFile,
                                                                "closest_hit_normals"
                                                                );
  box_matl->setClosestHitProgram( 0, box_ch );


  // Create GIs for each piece of geometry
  std::vector< optix::GeometryInstance > gis;
  gis.push_back( context_->createGeometryInstance( box, &box_matl, &box_matl + 1 ) );

  // Place all in group
  optix::GeometryGroup geometrygroup = context_->createGeometryGroup( );
  geometrygroup->setChildCount( static_cast< unsigned int >( gis.size( ) ) );
  geometrygroup->setChild( 0, gis[ 0 ] );
  geometrygroup->setAcceleration( context_->createAcceleration( "NoAccel", "NoAccel" ) );

  context_[ "top_object" ]->set( geometrygroup );

} // OptixSphereScene::_buildScene



} // namespace light
