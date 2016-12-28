#ifndef OptixBoxScene_hpp
#define OptixBoxScene_hpp


#include "OptixScene.hpp"
#include <vector>


namespace light
{


/////////////////////////////////////////////
/// \brief The OptixBoxScene class
///
/// \author Logan Barnes
/////////////////////////////////////////////
class OptixBoxScene : public OptixScene
{

public:

  ///////////////////////////////////////////////////////////////
  /// \brief OptixBoxScene
  ///////////////////////////////////////////////////////////////
  OptixBoxScene(
                int      width,
                int      height,
                unsigned vbo
                );


  ///////////////////////////////////////////////////////////////
  /// \brief ~OptixBoxScene
  ///////////////////////////////////////////////////////////////
  virtual
  ~OptixBoxScene( );

  void setDisplayType( int type );


protected:

private:

  void _buildGeometry ( );
  void _addLights ( );

  optix::Material sceneMaterial_;

  std::vector< optix::Program > materialPrograms_;



};


} // namespace light


#endif // OptixBoxScene_hpp
