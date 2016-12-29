#ifndef LightBenderIOHandler_hpp
#define LightBenderIOHandler_hpp


#include <string>
#include "io/ImguiOpenGLIOHandler.hpp"


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

class OptixScene;


/////////////////////////////////////////////
/// \brief The LightBenderIOHandler class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class LightBenderIOHandler : public shared::ImguiOpenGLIOHandler
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
  void _onRender ( const double alpha ) final;

  virtual
  void _onGuiRender( ) final;


  void _setScene( );


  std::unique_ptr< OptixScene > upScene_;

  int currentScene_;


};


} // namespace light


#endif // LightBenderIOHandler_hpp
