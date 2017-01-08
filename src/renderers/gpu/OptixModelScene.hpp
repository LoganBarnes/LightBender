#ifndef OptixModelScene_hpp
#define OptixModelScene_hpp


#include "OptixScene.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixModelScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixModelScene : public OptixScene
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixModelScene
  ///////////////////////////////////////////////////////////////
  OptixModelScene(
                     int      width,
                     int      height,
                     unsigned vbo
                     );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixModelScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixModelScene( );


protected:

private:

  void _buildGeometry ( );
  void _addLights ( );



};


} // namespace light


#endif // OptixModelScene_hpp
