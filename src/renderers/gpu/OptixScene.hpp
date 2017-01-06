#ifndef OptixScene_hpp
#define OptixScene_hpp


#include "OptixRenderer.hpp"
#include <vector>
#include <optixu/optixu_math_stream_namespace.h>


namespace light
{


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
  /// \brief createBoxPrimitive
  /// \param min
  /// \param max
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::Geometry createBoxPrimitive (
                                      optix::float3 min = optix::make_float3( -1.0f ),
                                      optix::float3 max = optix::make_float3(  1.0f )
                                      );


  ///////////////////////////////////////////////////////////////
  /// \brief createSpherePrimitive
  /// \param center
  /// \param radius
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::Geometry createSpherePrimitive (
                                         optix::float3 center = optix::make_float3( 0.0f ),
                                         float         radius = 1.0f
                                         );


  ///////////////////////////////////////////////////////////////
  /// \brief createQuadPrimitive
  /// \param anchor
  /// \param v1
  /// \param v2
  /// \return
  ///////////////////////////////////////////////////////////////
  optix::Geometry createQuadPrimitive (
                                       optix::float3 anchor =
                                         optix::make_float3( -1.0f, -1.0f, 0.0f ),
                                       optix::float3 v1     =
                                         optix::make_float3(  2.0f,  0.0f, 0.0f ),
                                       optix::float3 v2     =
                                         optix::make_float3(  0.0f,  2.0f, 0.0f )
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
                      optix::float3 translation   = optix::make_float3( 0.0f ),
                      optix::float3 scale         = optix::make_float3( 1.0f ),
                      float rotationAngle         = 0.0f,
                      optix::float3 rotationAxis  = optix::make_float3( 0.0f, 1.0f, 0.0f )
                      );


  ///////////////////////////////////////////////////////////////
  /// \brief createInputBuffer
  /// \param input
  /// \return
  ///////////////////////////////////////////////////////////////
  template< typename T >
  optix::Buffer createInputBuffer ( const std::vector< T > &input );


protected:

  optix::Material sceneMaterial_;

  std::vector< optix::Program > displayPrograms_;


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
