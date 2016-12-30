#ifndef OptixAdvancedScene_hpp
#define OptixAdvancedScene_hpp


#include "OptixScene.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixAdvancedScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixAdvancedScene : public OptixScene
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixAdvancedScene
  ///////////////////////////////////////////////////////////////
  OptixAdvancedScene(
                     int      width,
                     int      height,
                     unsigned vbo
                     );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixAdvancedScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixAdvancedScene( );


protected:

private:

  void _buildGeometry ( );
  void _addLights ( );



};


} // namespace light


#endif // OptixAdvancedScene_hpp
