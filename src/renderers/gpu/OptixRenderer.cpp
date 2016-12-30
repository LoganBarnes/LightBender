#include "OptixRenderer.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"

//#include <iostream>
//#include "glm/gtx/string_cast.hpp"

namespace light
{


///////////////////////////////////////////////////////////////
/// \brief OptixRenderer::OptixRenderer
///////////////////////////////////////////////////////////////
OptixRenderer::OptixRenderer(
                             int      width,
                             int      height,
                             unsigned vbo
                             )
  : RendererInterface( width, height )
  , background_color ( 0.0f, 0.0f, 0.0f )
  , error_color      ( 1.0f, 0.0f, 0.0f )
  , context_         ( optix::Context::create( ) )
  , pathTracing_     ( false )
  , frame_           ( 1u )
{

  // context
  context_->setRayTypeCount   ( 2 );
  context_->setEntryPointCount( 1 );
  context_->setStackSize      ( 4640 );

  context_[ "radiance_ray_type" ]->setUint ( 0 );
  context_[ "shadow_ray_type"   ]->setUint ( 1 );
  context_[ "scene_epsilon"     ]->setFloat( 1.e-2f );
  context_[ "frame_number"      ]->setUint ( frame_ );

  context_[ "eye" ]->setFloat( 0.0f, 0.0f,  0.0f );
  context_[ "U"   ]->setFloat( 1.0f, 0.0f,  0.0f );
  context_[ "V"   ]->setFloat( 0.0f, 1.0f,  0.0f );
  context_[ "W"   ]->setFloat( 0.0f, 0.0f, -1.0f );


  //
  // file with default programs
  //
  std::string defaultPtxFile(
                             light::RES_PATH
                             + "ptx/cudaLightBender_generated_Cameras.cu.ptx"
                             );

  //
  // Starting program to generate rays
  //
  context_->setRayGenerationProgram( 0, context_->createProgramFromPTXFile(
                                                                           defaultPtxFile,
                                                                           "pinhole_camera"
                                                                           ) );

  //
  // What to do when something messes up
  //
  context_[ "error_color" ]->setFloat( error_color.r, error_color.g, error_color.b );
  context_->setExceptionProgram( 0, context_->createProgramFromPTXFile(
                                                                       defaultPtxFile,
                                                                       "exception"
                                                                       ) );

  //
  // What to do when rays don't intersect with anything
  //
  context_[ "bg_color" ]->setFloat( background_color.r, background_color.g, background_color.b );
  context_->setMissProgram( 0, context_->createProgramFromPTXFile(
                                                                  defaultPtxFile,
                                                                  "miss"
                                                                  ) );

  optix::Buffer buffer;
  buffer = context_->createBufferFromGLBO( RT_BUFFER_OUTPUT, vbo );
  buffer->setFormat( RT_FORMAT_FLOAT4 );
  buffer->setSize(
                  static_cast< unsigned >( width ),
                  static_cast< unsigned >( height )
                  );

  context_[ "output_buffer" ]->set( buffer );

  setSqrtSamples( 1 );
  setCameraType ( 0 );

}



///////////////////////////////////////////////////////////////
/// \brief OptixRenderer::~OptixRenderer
///////////////////////////////////////////////////////////////
OptixRenderer::~OptixRenderer( )
{

  context_->destroy( );
  context_ = nullptr;

}



///////////////////////////////////////////////////////////////
/// \brief OptixBoxScene::setCameraType
/// \param type
///////////////////////////////////////////////////////////////
void
OptixRenderer::setCameraType( int type )
{

  std::string cameraPtxFile(
                            light::RES_PATH
                            + "ptx/cudaLightBender_generated_Cameras.cu.ptx"
                            );

  std::string camera( "pinhole_camera" );

  if ( type == 1 )
  {

    camera = "orthographic_camera";

  }

  if ( pathTracing_ )
  {

    camera = "pathtrace_" + camera;

  }

  context_->setRayGenerationProgram( 0, context_->createProgramFromPTXFile(
                                                                           cameraPtxFile,
                                                                           camera
                                                                           ) );

  resetFrameCount( );

} // OptixRenderer::setCameraType



void
OptixRenderer::setSqrtSamples( unsigned sqrtSamples )
{

  context_[ "sqrt_num_samples" ]->setUint( sqrtSamples );

}



///
/// \brief OptixRenderer::setPathTracing
/// \param pathTracing
///
void
OptixRenderer::setPathTracing( int pathTracing )
{

  pathTracing_ = pathTracing;

}



///
/// \brief OptixRenderer::resize
/// \param w
/// \param h
///
void
OptixRenderer::resize(
                      int w,
                      int h
                      )
{

  width_  = static_cast< unsigned >( w );
  height_ = static_cast< unsigned >( h );

  resetFrameCount( );

}



///
/// \brief OptixRenderer::renderWorld
///
void
OptixRenderer::renderWorld( const graphics::Camera &camera )
{

  glm::vec3 U, V, W;
  glm::vec3 eye( camera.getEye( ) );

  camera.buildRayBasisVectors( &U, &V, &W );

  context_[ "eye" ]->setFloat( eye.x, eye.y, eye.z );
  context_[ "U"   ]->setFloat(   U.x,   U.y,   U.z );
  context_[ "V"   ]->setFloat(   V.x,   V.y,   V.z );
  context_[ "W"   ]->setFloat(   W.x,   W.y,   W.z );

  context_[ "frame_number" ]->setUint ( frame_ );

  optix::Buffer buffer = context_[ "output_buffer" ]->getBuffer( );
  RTsize buffer_width, buffer_height;
  buffer->getSize( buffer_width, buffer_height );

  context_->launch(
                   0,
                   static_cast< unsigned >( buffer_width  ),
                   static_cast< unsigned >( buffer_height )
                   );

  ++frame_;

} // OptixRenderer::RenderWorld



///
/// \brief OptixRenderer::getBuffer
/// \return
///
optix::Buffer
OptixRenderer::getBuffer( )
{

  return context_[ "output_buffer" ]->getBuffer( );

}



///
/// \brief OptixRenderer::resetFrameCount
///
void
OptixRenderer::resetFrameCount( )
{

  frame_ = 1;

}



} // namespace light
