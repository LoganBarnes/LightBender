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
#include "OptixBoxScene.hpp"
#include "OptixSphereScene.hpp"


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
  , currentScene_       ( 0 )
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
  std::vector< float > vboData =
  {
    -1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0
  };

  upGLWrapper_->addUVBuffer(
                            "screenBuffer",
                            "fullscreenProgram",
                            vboData.data( ),
                            vboData.size( )
                            );

  upGLWrapper_->addTextureArray( "rayTex", defaultWidth, defaultHeight, 0 );

  // First allocate the memory for the GL buffer, then attach it to OptiX.
  upGLWrapper_->addBufferNoVAO< float >(
                                        "renderBuffer",
                                        sizeof( optix::float4 ) * defaultWidth * defaultHeight,
                                        0,
                                        GL_STREAM_DRAW
                                        );


  _setScene( );


  //
  // camera
  //
  upCamera_->setAspectRatio( defaultWidth * 1.0f / defaultHeight );
  upCamera_->updateOrbit( 20.0f, 45.0f, -30.0f );

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
LightBenderIOHandler::_onRender( const double )
{

  upGLWrapper_->clearWindow( );

  upGLWrapper_->useProgram  ( "fullscreenProgram" );

  if ( upRenderer_ )
  {

    upRenderer_->renderWorld( *upCamera_ );


    optix::Buffer buffer = upRenderer_->getBuffer( );

    RTsize elementSize = buffer->getElementSize( );

    int alignmentSize = 1;

    if ( ( elementSize % 8 ) == 0 )
    {
      alignmentSize = 8;
    }
    else
    if ( ( elementSize % 4 ) == 0 )
    {
      alignmentSize = 4;
    }
    else
    if ( ( elementSize % 2 ) == 0 )
    {
      alignmentSize = 2;
    }

    RTsize bufWidth, bufHeight;
    buffer->getSize( bufWidth, bufHeight );


    upGLWrapper_->bindBufferToTexture(
                                      "rayTex",
                                      buffer->getGLBOId( ),
                                      alignmentSize,
                                      bufWidth,
                                      bufHeight
                                      );

  }

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

  upGLWrapper_->setTextureUniform(
                                  "fullscreenProgram",
                                  "tex",
                                  "rayTex",
                                  0
                                  );

  upGLWrapper_->renderBuffer( "screenBuffer", 4, GL_TRIANGLE_STRIP );

} // LightBenderIOHandler::onRender



void
LightBenderIOHandler::_onGuiRender( )
{

  bool alwaysTrue = true;

  ImGui::SetNextWindowSize( ImVec2( 350, 250 ), /*alwaysTrue*/ ImGuiSetCond_FirstUseEver );

  ImGui::Begin( "Light Bender Settings", &alwaysTrue );

  //
  // FPS
  //
  ImGui::Text(
              "Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO( ).Framerate,
              ImGui::GetIO( ).Framerate
              );

  ImGui::Separator( );

  glm::vec3 eye ( upCamera_->getEye( )  );
  glm::vec3 look( upCamera_->getLook( ) );
  ImGui::Text( "EYE:  %2.2f, %2.2f, %2.2f",    eye.x,  eye.y,  eye.z  );
  ImGui::Text( "LOOK:  %1.2f,  %1.2f,  %1.2f", look.x, look.y, look.z );


  //
  // Scene selction
  //
  if ( ImGui::CollapsingHeader( "Scene Selection", "scene", false, true ) )
  {

    int oldScene = currentScene_;

    ImGui::Combo( "", &currentScene_, " Box \0 Other Box \0\0" );

    if ( oldScene != currentScene_ )
    {

      _setScene( );

    }

  }


  //
  // Control listing
  //
  if ( ImGui::CollapsingHeader( "Controls", "controls", false, true ) )
  {

    ImGui::Text(
                "Camera Movement:\n\n"
                "    Left Mouse      -    Rotate\n" \
                "    Right Mouse     -    Zoom\n" \
                "    Middle Scroll   -    Zoom\n"
                );

  }

  ImGui::End( );

} // LightBenderIOHandler::onGuiRender



///
/// \brief LightBenderIOHandler::_setScene
///
void
LightBenderIOHandler::_setScene( )
{

  if ( upRenderer_ )
  {

    upRenderer_.reset( );

  }

  switch ( currentScene_ )
  {

  case 0:

    upRenderer_
      = std::unique_ptr< OptixRenderer >( new OptixBoxScene(
                                                            defaultWidth,
                                                            defaultHeight,
                                                            upGLWrapper_->getBuffer( "renderBuffer" )
                                                            ) );
    break;

  case 1:

    upRenderer_
      = std::unique_ptr< OptixRenderer >( new OptixSphereScene(
                                                               defaultWidth,
                                                               defaultHeight,
                                                               upGLWrapper_->getBuffer(
                                                                                       "renderBuffer" )
                                                               ) );
    break;

  default:
    throw std::runtime_error( "Unknown scene" );
    break;

  } // switch

} // LightBenderIOHandler::_setScene



} // namespace light
