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

class RendererInterface;


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
  void onRender ( const double alpha ) final;

  virtual
  void onGuiRender( ) final;


  std::unique_ptr< RendererInterface > upRenderer_;



};


} // namespace light


#endif // LightBenderIOHandler_hpp
