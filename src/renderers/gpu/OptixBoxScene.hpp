#ifndef OptixBoxScene_hpp
#define OptixBoxScene_hpp


#include "OptixRenderer.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixBoxScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixBoxScene : public OptixRenderer
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixBoxScene
  ///////////////////////////////////////////////////////////////
  OptixBoxScene(
                int      width,
                int      height,
                unsigned vbo
                );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixBoxScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixBoxScene( );


protected:

private:

  void _buildScene ( );



};


} // namespace light


#endif // OptixBoxScene_hpp
