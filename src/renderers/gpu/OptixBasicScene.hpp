#ifndef OptixBasicScene_hpp
#define OptixBasicScene_hpp


#include "OptixScene.hpp"
#include <vector>


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


protected:

private:

  void _buildGeometry ( );
  void _addLights ( );



};


} // namespace light


#endif // OptixBasicScene_hpp
