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
#include "io/ImguiCallback.hpp"
#include "imgui.h"

// project
#include "LightBenderCallback.hpp"
#include "LightBenderConfig.hpp"
#include "OptixRenderer.hpp"


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
  : ImguiOpenGLIOHandler( world, true, defaultWidth, defaultHeight, false )
{

  std::unique_ptr< graphics::Callback > upCallback( new LightBenderCallback( *this ) );

  imguiCallback_->setCallback( std::move( upCallback ) );


  //
  // OpenGL
  //

  upGLWrapper_->init( );

  upGLWrapper_->setViewportSize( defaultWidth, defaultHeight );

  std::string vertShader = SHADER_PATH + "screenSpace/shader.vert";
  std::string fragShader = SHADER_PATH + "screenSpace/shader.frag";

  upGLWrapper_->addProgram(
                           "fullscreenProgram",
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
                            "fullscreenProgram",
                            vbo.data( ),
                            vbo.size( )
                            );

  upGLWrapper_->addTextureArray( "rayTex", defaultWidth, defaultHeight, 0 );


  upRenderer_ = std::unique_ptr< RendererInterface >( new OptixRenderer(
                                                                        defaultWidth,
                                                                        defaultHeight,
                                                                        upGLWrapper_->getVBO(
                                                                                             "screenBuffer" )
                                                                        ) );


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
{}



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

  upRenderer_->renderWorld( *upCamera_ );

//  optix::Buffer context

//  glBindTexture( GL_TEXTURE_2D, upGLWrapper_->getTexture( "rayTex" ) );
//  glBindBuffer( GL_PIXEL_UNPACK_BUFFER, upGLWrapper_->getVBO( "screenBuffer" ) );

//  RTsize elmt_size = buffer

  upGLWrapper_->clearWindow( );

  upGLWrapper_->useProgram  ( "fullscreenProgram" );

  glm::vec2 screenSize(
                       upGLWrapper_->getViewportWidth( ),
                       upGLWrapper_->getViewportHeight( )
                       );

  upGLWrapper_->setFloatUniform(
                                "fullscreenProgram",
                                "screenSize",
                                glm::value_ptr( screenSize ),
                                2
                                );

  upGLWrapper_->renderBuffer( "screenBuffer", 4, GL_TRIANGLE_STRIP );

} // LightBenderIOHandler::onRender



void
LightBenderIOHandler::onGuiRender( )
{

  bool alwaysTrue = true;

  ImGui::SetNextWindowSize( ImVec2( 200, 100 ), alwaysTrue );
  ImGui::Begin( "Another Window", &alwaysTrue );
  ImGui::Text( "Hello" );
  ImGui::End( );

}



} // namespace light
