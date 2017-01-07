#ifndef OptixFileScene_hpp
#define OptixFileScene_hpp


#include "OptixScene.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixFileScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixFileScene : public OptixScene
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixFileScene
  ///////////////////////////////////////////////////////////////
  OptixFileScene(
                 int                width,
                 int                height,
                 unsigned           vbo,
                 const std::string &filename
                 );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixFileScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixFileScene( );


protected:

private:


};


} // namespace light


#endif // OptixFileScene_hpp
