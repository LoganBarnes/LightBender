#ifndef OptixSphereScene_hpp
#define OptixSphereScene_hpp


#include "OptixScene.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixSphereScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixSphereScene : public OptixScene
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixSphereScene
  ///////////////////////////////////////////////////////////////
  OptixSphereScene(
                   int      width,
                   int      height,
                   unsigned vbo
                   );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixSphereScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixSphereScene( );


protected:

private:

  void _buildGeometry ( );
  void _addLights ( );



};


} // namespace light


#endif // OptixSphereScene_hpp
