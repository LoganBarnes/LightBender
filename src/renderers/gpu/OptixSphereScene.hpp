#ifndef OptixSphereScene_hpp
#define OptixSphereScene_hpp


#include "OptixRenderer.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixSphereScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixSphereScene : public OptixRenderer
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

  void _buildScene ( );



};


} // namespace light


#endif // OptixSphereScene_hpp
