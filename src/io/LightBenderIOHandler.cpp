#include "LightBenderIOHandler.hpp"

// system
#include <vector>

// shared
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "graphics/glfw/GlfwWrapper.hpp"
#include "graphics/opengl/OpenGLWrapper.hpp"
#include "graphics/Camera.hpp"

// project
#include "LightBenderCallback.hpp"
#include "LightBenderConfig.hpp"


namespace light
{


namespace
{

constexpr int defaultWidth  = 1080;
constexpr int defaultHeight = 720;

}


/////////////////////////////////////////////
/// \brief LightBender::LightBender
///
/// \author Logan Barnes
/////////////////////////////////////////////
LightBenderIOHandler::LightBenderIOHandler( shared::World &world )
  : OpenGLIOHandler( world, true, defaultWidth, defaultHeight, false )
{

  std::unique_ptr< shared::SharedCallback > upCallback( new LightBenderCallback( *this ) );

  upGlfwWrapper_->setCallback( std::move( upCallback ) );


  //
  // OpenGL
  //

  upGLWrapper_->init( );

  upGLWrapper_->setViewportSize( defaultWidth, defaultHeight );

  std::string vertShader = SHADER_PATH + "screenSpace/shader.vert";
  std::string fragShader = SHADER_PATH + "blending/shader.frag";

  upGLWrapper_->addProgram(
                           "path",
                           vertShader.c_str( ),
                           fragShader.c_str( )
                           );

  // temporary
  std::vector< float > vbo =
  {
    -1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0
  };

  upGLWrapper_->addUVBuffer(
                            "screenBuffer",
                            "path",
                            vbo.data( ),
                            vbo.size( )
                            );


  //
  // camera
  //

  upCamera_->setAspectRatio( defaultWidth * 1.0f / defaultHeight );
  upCamera_->updateOrbit( 7.0f, 0.0f, 0.0f );



}



/////////////////////////////////////////////
/// \brief LightBender::~LightBender
///
/// \author Logan Barnes
/////////////////////////////////////////////
LightBenderIOHandler::~LightBenderIOHandler( )
{
}



void
LightBenderIOHandler::rotateCamera(
                                double deltaX,
                                double deltaY
                                )
{

  upCamera_->updateOrbit( 0.f, static_cast< float >( deltaX ), static_cast< float >( deltaY ) );

}



void
LightBenderIOHandler::zoomCamera( double deltaZ )
{

  upCamera_->updateOrbit( static_cast< float >( deltaZ * 0.25 ), 0.f, 0.f );

}




void
LightBenderIOHandler::resize(
                          int w,
                          int h
                          )
{

  upGLWrapper_->setViewportSize( w, h );
  upCamera_->setAspectRatio( w * 1.0f / h );

}



/////////////////////////////////////////////
/// \brief LightBender::onRender
/// \param alpha
///
/// \author Logan Barnes
/////////////////////////////////////////////
void
LightBenderIOHandler::onRender( const double )
{

  upGLWrapper_->clearWindow( );

} // LightBenderIOHandler::onRender




} // namespace light
