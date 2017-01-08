#ifndef OptixBasicScene_hpp
#define OptixBasicScene_hpp


#include "OptixScene.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixBasicScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixBasicScene : public OptixScene
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixBasicScene
  ///////////////////////////////////////////////////////////////
  OptixBasicScene(
                  int      width,
                  int      height,
                  unsigned vbo
                  );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixBasicScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixBasicScene( );


  ///////////////////////////////////////////////////////////////
  /// \brief renderSceneGui
  ///
  ///        Allows for specific manipulation of each scene
  ///////////////////////////////////////////////////////////////
  virtual
  void renderSceneGui( );


protected:

private:

  void _buildGeometry ( );
  void _addLights ( );



};


} // namespace light


#endif // OptixBasicScene_hpp
