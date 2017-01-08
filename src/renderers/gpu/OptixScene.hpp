#ifndef OptixScene_hpp
#define OptixScene_hpp


#include "OptixRenderer.hpp"
#include <vector>
#include <cstring>
#include "optixMod/optix_math_stream_namespace_mod.h"


namespace light
{


struct ShapeGroup
{

  optix::GeometryGroup group;
  std::vector< optix::Geometry > geometries;
  std::vector< optix::Material > materials;

  optix::Matrix4x4 transform
  {
    optix::Matrix4x4::identity( )
  };

  std::string builderAccel
  {
    "NoAccel"
  };

  std::string traverserAccel
  {
    "NoAccel"
  };

};


/////////////////////////////////////////////
/// \brief The OptixScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixScene : public OptixRenderer
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixScene
  ///////////////////////////////////////////////////////////////
  OptixScene(
             int      width,
             int      height,
             unsigned vbo
             );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixScene( );


  ///////////////////////////////////////////////////////////////
  /// \brief setDisplayType
  /// \param type
  ///////////////////////////////////////////////////////////////
  void setDisplayType ( int type );


  ///////////////////////////////////////////////////////////////
  /// \brief setMaxBounces
  /// \param bounces
  ///////////////////////////////////////////////////////////////
  void setMaxBounces ( unsigned bounces );


  ///////////////////////////////////////////////////////////////
  /// \brief setFirstBounce
  /// \param bounce
  ///////////////////////////////////////////////////////////////
  void setFirstBounce ( unsigned bounce );


  ///////////////////////////////////////////////////////////////
  /// \brief createBoxPrimitive
  /// \param min
  /// \param max
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::Geometry createBoxPrimitive (
                                      optix::float3 min = optix::float3 { -1.0f, -1.0f, -1.0f },
                                      optix::float3 max = optix::float3 {  1.0f,  1.0f,  1.0f }
                                      );


  ///////////////////////////////////////////////////////////////
  /// \brief createSpherePrimitive
  /// \param center
  /// \param radius
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::Geometry createSpherePrimitive (
                                         optix::float3 center = optix::float3 { 0.0f, 0.0f, 0.0f },
                                         float radius = 1.0f
                                         );


  ///////////////////////////////////////////////////////////////
  /// \brief createQuadPrimitive
  /// \param anchor
  /// \param v1
  /// \param v2
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::Geometry createQuadPrimitive (
                                       optix::float3 anchor = optix::float3 { -1.0f, -1.0f, 0.0f },
                                       optix::float3 v1     = optix::float3 {  2.0f,  0.0f, 0.0f },
                                       optix::float3 v2     = optix::float3 {  0.0f,  2.0f, 0.0f }
                                       );


  ///////////////////////////////////////////////////////////////
  /// \brief createMaterial
  /// \param closestHitProgram
  /// \param anyHitProgram
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::Material createMaterial (
                                  optix::Program closestHitProgram,
                                  optix::Program anyHitProgram
                                  );


  ///////////////////////////////////////////////////////////////
  /// \brief createGeomGroup
  /// \param geometries
  /// \param materials
  /// \param builderAccel
  /// \param traverserAccel
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::GeometryGroup createGeomGroup (
                                        const std::vector< optix::Geometry > &geometries,
                                        const std::vector< optix::Material > &materials,
                                        const std::string                    &builderAccel,
                                        const std::string                    &traverserAccel
                                        );


  ///////////////////////////////////////////////////////////////
  /// \brief createShapeGeomGroup
  /// \param pShape
  ///////////////////////////////////////////////////////////////
  void createShapeGeomGroup ( ShapeGroup *pShape );


  ///////////////////////////////////////////////////////////////
  /// \brief attachToGroup
  /// \param group
  /// \param geomGroup
  /// \param translation
  /// \param scale
  /// \param rotationAngle
  /// \param rotationAxis
  ///////////////////////////////////////////////////////////////
  void attachToGroup (
                      optix::Group group,
                      optix::GeometryGroup geomGroup,
                      unsigned childNum,
                      optix::float3 translation   = optix::float3 { 0.0f, 0.0f, 0.0f },
                      optix::float3 scale         = optix::float3 { 1.0f, 1.0f, 1.0f },
                      float rotationAngle         = 0.0f,
                      optix::float3 rotationAxis  = optix::float3 { 0.0f, 1.0f, 0.0f }
                      );



  ///////////////////////////////////////////////////////////////
  /// \brief attachToGroup
  /// \param group
  /// \param geomGroup
  /// \param childNum
  /// \param M
  ///////////////////////////////////////////////////////////////
  void attachToGroup (
                      optix::Group         group,
                      optix::GeometryGroup geomGroup,
                      unsigned             childNum,
                      optix::Matrix4x4     M
                      );



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
  ShapeGroup createShapeGroup (
                               const std::vector< optix::Geometry > &geometries,
                               const std::vector< optix::Material > &materials,
                               const std::string &builderAccel   = "NoAccel",
                               const std::string &traverserAccel = "NoAccel",
                               optix::float3 translation   = optix::float3 { 0.0f, 0.0f, 0.0f },
                               optix::float3 scale         = optix::float3 { 1.0f, 1.0f, 1.0f },
                               float rotationAngle         = 0.0f,
                               optix::float3 rotationAxis  = optix::float3 { 0.0f, 1.0f, 0.0f }
                               );


  ///////////////////////////////////////////////////////////////
  /// \brief createInputBuffer
  /// \param input
  /// \return
  ///////////////////////////////////////////////////////////////
  template< typename T >
  optix::Buffer createInputBuffer ( const std::vector< T > &input );


  ///////////////////////////////////////////////////////////////
  /// \brief renderSceneGui
  ///
  ///        Allows for specific manipulation of each scene
  ///////////////////////////////////////////////////////////////
  virtual
  void renderSceneGui( ) = 0;


protected:

  optix::Material sceneMaterial_;

  std::vector< optix::Program > materialPrograms_;

  std::vector< ShapeGroup > shapes_;


private:

};



///
/// \brief OptixScene::createInputBuffer
/// \param input
/// \return
///
template< typename T >
optix::Buffer
OptixScene::createInputBuffer( const std::vector< T > &input )
{

  optix::Buffer buffer = context_->createBuffer( RT_BUFFER_INPUT );

  buffer->setFormat( RT_FORMAT_USER );
  buffer->setElementSize( sizeof( T ) );
  buffer->setSize( input.size( ) );
  memcpy( buffer->map( ), input.data( ), input.size( ) * sizeof( T ) );
  buffer->unmap( );

  return buffer;

}



} // namespace light


#endif // OptixScene_hpp
