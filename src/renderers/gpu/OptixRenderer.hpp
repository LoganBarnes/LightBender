#ifndef OptixRenderer_hpp
#define OptixRenderer_hpp


#include "optixu/optixpp_namespace.h"
#include "RendererInterface.hpp"
#include "glm/glm.hpp"


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixRenderer class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixRenderer : public RendererInterface
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixRenderer
  ///////////////////////////////////////////////////////////////
  OptixRenderer(
                int      width,
                int      height,
                unsigned vbo
                );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixRenderer
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixRenderer( );


  void setCameraType  ( int type );
  void setSqrtSamples ( unsigned sqrtSamples );
  void setPathTracing ( bool pathTracing );


  virtual
  void resize (
               int w,
               int h
               ) final;

  virtual
  void renderWorld ( const graphics::Camera &camera ) final;


  optix::Buffer getBuffer ( );

  void resetFrameCount ( );


  glm::vec3 background_color;
  glm::vec3 error_color;


protected:

  optix::Context context_;


private:

//  bool   m_camera_changed;
//  bool   m_use_pbo_buffer;
//  int    m_num_devices;
//  bool   m_cpu_rendering_enabled;

  bool pathTracing_;
  unsigned frame_;

  unsigned width_;
  unsigned height_;



};


} // namespace light


#endif // OptixRenderer_hpp
