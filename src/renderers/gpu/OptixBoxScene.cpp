#include "OptixBoxScene.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::OptixBoxScene
///////////////////////////////////////////////////////////////
OptixBoxScene::OptixBoxScene(
                             int      width,
                             int      height,
                             unsigned vbo
                             )
  : RendererInterface( width, height )
  , background_color ( 0.0f )
  , error_color      ( 1.0f, 0.0f, 0.0f )
  , context_         ( optix::Context::create( ) )

{

  _buildScene( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::~OptixBoxScene
///////////////////////////////////////////////////////////////
OptixBoxScene::~OptixBoxScene( )
{}




} // namespace light
