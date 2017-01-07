#include "OptixFileScene.hpp"
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "commonStructs.h"


namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixFileScene::OptixFileScene
///////////////////////////////////////////////////////////////
OptixFileScene::OptixFileScene(
                               int                width,
                               int                height,
                               unsigned           vbo,
                               const std::string &
                               )
  : OptixScene( width, height, vbo )
{

  context_->validate( );
  context_->compile( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixFileScene::~OptixFileScene
///////////////////////////////////////////////////////////////
OptixFileScene::~OptixFileScene( )
{}




} // namespace light
