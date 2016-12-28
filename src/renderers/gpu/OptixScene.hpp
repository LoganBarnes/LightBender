#ifndef OptixScene_hpp
#define OptixScene_hpp


#include "OptixRenderer.hpp"
#include <vector>


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
//  void setDisplayType( int type );


  ///////////////////////////////////////////////////////////////
  optix::Geometry createBoxPrimitive (
                                     optix::float3 min,
                                     optix::float3 max
                                     );


  ///////////////////////////////////////////////////////////////
  optix::Geometry createSpherePrimitive (
                                        optix::float3 center,
                                        float         radius
                                        );


  ///////////////////////////////////////////////////////////////
  optix::Geometry createQuadPrimitive (
                                      optix::float3 anchor,
                                      optix::float3 v1,
                                      optix::float3 v2
                                      );


protected:

private:

  // void _addLights ( );

  // optix::Material sceneMaterial_;

  // std::vector< optix::Program > materialPrograms_;



};


} // namespace light


#endif // OptixScene_hpp
