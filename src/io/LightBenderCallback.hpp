#ifndef LightBenderCallback_hpp
#define LightBenderCallback_hpp

#include "io/SharedCallback.hpp"

namespace light
{

class LightBenderIOHandler;


class LightBenderCallback : public shared::SharedCallback
{
public:

  LightBenderCallback( LightBenderIOHandler &handler );

  virtual
  ~LightBenderCallback( );


  virtual
  void handleWindowSize (
                         GLFWwindow *pWindow,
                         int         width,
                         int         height
                         );

  virtual
  void handleMouseButton (
                          GLFWwindow *pWindow,
                          int         button,
                          int         action,
                          int         mods
                          );

  virtual
  void handleKey (
                  GLFWwindow *pWindow,
                  int         key,
                  int         scancode,
                  int         action,
                  int         mods
                  );

  virtual
  void handleCursorPosition (
                             GLFWwindow *pWindow,
                             double      xpos,
                             double      ypos
                             );

  virtual
  void handleScroll (
                     GLFWwindow *pWindow,
                     double      xoffset,
                     double      yoffset
                     );


private:

  LightBenderIOHandler &handler_;

  bool leftMouseDown_;
  bool rightMouseDown_;

  bool shiftDown_;

  double prevX_;
  double prevY_;

};


} //  namespace light


#endif // LightBenderCallback_hpp
