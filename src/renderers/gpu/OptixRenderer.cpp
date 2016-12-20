#include "OptixRenderer.hpp"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"


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
  , background_color ( 0.0f )
  , error_color      ( 1.0f, 0.0f, 0.0f )
  , context_         ( optix::Context::create( ) )

{

  // context
  context_->setRayTypeCount( 2 );
  context_->setEntryPointCount( 1 );
  context_->setStackSize( 4640 );

  context_[ "radiance_ray_type"   ]->setUint ( 0 );
  context_[ "scene_epsilon"       ]->setFloat( 1.e-2f );

  context_[ "eye" ]->setFloat( 0.0f, 0.0f,  0.0f );
  context_[ "U"   ]->setFloat( 1.0f, 0.0f,  0.0f );
  context_[ "V"   ]->setFloat( 0.0f, 1.0f,  0.0f );
  context_[ "W"   ]->setFloat( 0.0f, 0.0f, -1.0f );


  //
  // file with default programs
  //
  std::string defaultPtxFile = light::RES_PATH
                               + "ptx/cudaLightBender_generated_OptixRenderer.cu.ptx";

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
  context_[ "bad_color" ]->setFloat( error_color.r, error_color.g, error_color.b );
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
                  static_cast< unsigned >( width_  ),
                  static_cast< unsigned >( height_ )
                  );

  context_[ "output_buffer" ]->set( buffer );

  context_->validate( );

}



///////////////////////////////////////////////////////////////
/// \brief OptixRenderer::~OptixRenderer
///////////////////////////////////////////////////////////////
OptixRenderer::~OptixRenderer( )
{

  context_->destroy( );
  context_ = nullptr;

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

  context_->launch(
                   0,
                   static_cast< unsigned >( width_  ),
                   static_cast< unsigned >( height_ )
                   );

} // OptixRenderer::RenderWorld



} // namespace light
