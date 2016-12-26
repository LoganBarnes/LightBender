#include "LightBenderCallback.hpp"
#include "GLFW/glfw3.h"
#include "LightBenderIOHandler.hpp"


namespace light
{



LightBenderCallback::LightBenderCallback( LightBenderIOHandler &handler )
  : shared::SharedCallback( )
  , handler_( handler )
  , leftMouseDown_( false )
  , rightMouseDown_( false )
  , shiftDown_( false )
{}


LightBenderCallback::~LightBenderCallback( )
{}



///
/// \brief LightBenderCallback::handleWindowSize
/// \param pWindow
/// \param width
/// \param height
///
void
LightBenderCallback::handleWindowSize(
                                      GLFWwindow*,
                                      int width,
                                      int height
                                      )
{

  handler_.resize( width, height );

}



void
LightBenderCallback::handleMouseButton(
                                       GLFWwindow *pWindow,
                                       int         button,
                                       int         action,
                                       int
                                       )
{

  if ( button == GLFW_MOUSE_BUTTON_1 )
  {

    if ( action == GLFW_PRESS )
    {

      leftMouseDown_ = true;
      glfwGetCursorPos( pWindow, &prevX_, &prevY_ );

    }
    else
    {

      leftMouseDown_ = false;

    }
  }
  else
  if ( button == GLFW_MOUSE_BUTTON_2 )
  {

    if ( action == GLFW_PRESS )
    {

      rightMouseDown_ = true;
      glfwGetCursorPos( pWindow, &prevX_, &prevY_ );

    }
    else
    {

      rightMouseDown_ = false;

    }

  }

} // handleMouseButton



void
LightBenderCallback::handleKey(
                               GLFWwindow *pWindow,
                               int         key,
                               int,
                               int         action,
                               int
                               )
{
  switch ( key )
  {

  case GLFW_KEY_ESCAPE:

    if ( action == GLFW_RELEASE )
    {

      glfwSetWindowShouldClose( pWindow, GL_TRUE );

    }

    break;

  case GLFW_KEY_LEFT_SHIFT:
  case GLFW_KEY_RIGHT_SHIFT:

    if ( action == GLFW_PRESS )
    {

      shiftDown_ = true;

    }
    else
    {

      shiftDown_ = false;

    }

    break;

  default:
    break;

  } // switch

} // handleKey



//void LightBenderCallback::handleCursorPosition(GLFWwindow *window, double xpos, double ypos)
void
LightBenderCallback::handleCursorPosition(
                                          GLFWwindow*,
                                          double xpos,
                                          double ypos
                                          )
{

  if ( leftMouseDown_ )
  {

    handler_.rotateCamera( prevX_ - xpos, prevY_ - ypos );

  }
  else if ( rightMouseDown_ )
  {

    handler_.zoomCamera( prevY_ - ypos );

  }

  prevX_ = xpos;
  prevY_ = ypos;

}



//void LightBenderCallback::handleScroll(GLFWwindow* widnow, double xoffset, double yoffset)
void
LightBenderCallback::handleScroll(
                                  GLFWwindow*,
                                  double,
                                  double yoffset
                                  )
{

  handler_.zoomCamera( yoffset );

}



} //  namespace light
