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
                  int               width,
                  int               height,
                  unsigned          vbo,
                  const std::string &filename
                  );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixModelScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixModelScene( );


protected:

private:

  void _buildScene ( const std::string &filename );



};


} // namespace light


#endif // OptixModelScene_hpp
