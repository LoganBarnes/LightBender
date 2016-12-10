#ifndef LightBenderIOHandler_hpp
#define LightBenderIOHandler_hpp


#include <string>
#include "io/OpenGLIOHandler.hpp"


namespace graphics
{

class OpenGLWrapper;
class Camera;

}


namespace shared
{

class World;

}


namespace light
{

class PathTracer;


/////////////////////////////////////////////
/// \brief The LightBenderIOHandler class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class LightBenderIOHandler : public shared::OpenGLIOHandler
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief LightBender
  ///////////////////////////////////////////////////////////////
  LightBenderIOHandler( shared::World &world );


  ///////////////////////////////////////////////////////////////
  /// \brief ~LightBender
  ///////////////////////////////////////////////////////////////
  virtual
  ~LightBenderIOHandler( );


  void rotateCamera (
                     double deltaX,
                     double deltaY
                     );


  void zoomCamera ( double deltaZ );


  void resize( int w, int h );


protected:

private:

  virtual
  void onRender ( const double alpha ) final;



};


} // namespace light


#endif // LightBenderIOHandler_hpp
