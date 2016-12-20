#ifndef OptixRenderer_hpp
#define OptixRenderer_hpp


#include "optix/optixu/optixpp_namespace.h"
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
  ~OptixRenderer( );

  virtual
  void resize (
               int w,
               int h
               ) final;

  virtual
  void renderWorld ( const graphics::Camera &camera ) final;


  glm::vec3 background_color;
  glm::vec3 error_color;


protected:

  optix::Context context_;


private:


//  bool   m_camera_changed;
//  bool   m_use_pbo_buffer;
//  int    m_num_devices;
//  bool   m_cpu_rendering_enabled;

  unsigned int width_;
  unsigned int height_;



};


} // namespace light


#endif // OptixRenderer_hpp
