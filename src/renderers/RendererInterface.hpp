#ifndef RendererInterface_hpp
#define RendererInterface_hpp


namespace graphics
{

class Camera;

}


namespace light
{

class WorldScene;


struct RenderPackage
{

  graphics::Camera *pCamera;

};


/////////////////////////////////////////////
/// \brief The RendererInterface class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class RendererInterface
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief LightBender
  ///////////////////////////////////////////////////////////////
  RendererInterface(
                    int width,
                    int height
                    )
    : width_ ( width  )
    , height_( height )
  {}

  virtual
  ~RendererInterface( ) {}

  virtual
  void resize (
               int w,
               int h
               ) = 0;

  virtual
  void renderWorld ( const graphics::Camera &camera ) = 0;


protected:

  int width_;
  int height_;


private:

};


} // namespace light


#endif // RendererInterface_hpp
