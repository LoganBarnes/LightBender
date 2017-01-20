#include "OptixRenderer.hpp"
#include <stdexcept>
#include <fstream>
#include <vector>
#include <cstring>
#include <random>
#include <limits>
#include "glad/glad.h"
#include "LightBenderConfig.hpp"
#include "graphics/Camera.hpp"
#include "optixMod/optix_math_stream_namespace_mod.h"


namespace
{

std::random_device               rd;
std::mt19937                     gen( rd( ) );
std::uniform_int_distribution< unsigned > dis( 0, std::numeric_limits< unsigned >::max( ) );

}


// Error check/report helper for users of the C API
#define RT_CHECK_ERROR( func )                                     \
  do {                                                             \
    RTresult code = func;                                          \
    if( code != RT_SUCCESS )                                       \
      throw light::APIError( code, __FILE__, __LINE__ );           \
  } while(0)



namespace light
{

// Exeption to be thrown by RT_CHECK_ERROR macro
struct APIError
{
    APIError( RTresult c, const std::string& f, int l )
        : code( c ), file( f ), line( l ) {}
    RTresult     code;
    std::string  file;
    int          line;
};


namespace
{


void SavePPM(const unsigned char *Pix, const char *fname, int wid, int hgt, int chan)
{
    if( Pix==NULL || wid < 1 || hgt < 1 )
        throw std::runtime_error( "Image is ill-formed. Not saving" );

    if( chan != 1 && chan != 3 && chan != 4 )
        throw std::runtime_error( "Attempting to save image with channel count != 1, 3, or 4.");

    std::ofstream OutFile(fname, std::ios::out | std::ios::binary);
    if(!OutFile.is_open())
        throw std::runtime_error( "Could not open file for SavePPM" );

    bool is_float = false;
    OutFile << 'P';
    OutFile << ((chan==1 ? (is_float?'Z':'5') : (chan==3 ? (is_float?'7':'6') : '8'))) << std::endl;
    OutFile << wid << " " << hgt << std::endl << 255 << std::endl;

    OutFile.write(reinterpret_cast<char*>(const_cast<unsigned char*>( Pix )), wid * hgt * chan * (is_float ? 4 : 1));

    OutFile.close();
}

//
// stupid thirdparty code causing warnings
//
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif

void displayBufferPPM( const char *filename, RTbuffer buffer)
{
    GLsizei width, height;
    RTsize buffer_width, buffer_height;

    GLvoid* imageData;
    RT_CHECK_ERROR( rtBufferMap( buffer, &imageData) );

    RT_CHECK_ERROR( rtBufferGetSize2D(buffer, &buffer_width, &buffer_height) );
    width  = static_cast<GLsizei>(buffer_width);
    height = static_cast<GLsizei>(buffer_height);

    std::vector<unsigned char> pix( static_cast< unsigned >( width * height * 3 ) );

    RTformat buffer_format;
    RT_CHECK_ERROR( rtBufferGetFormat(buffer, &buffer_format) );

    switch(buffer_format) {
        case RT_FORMAT_UNSIGNED_BYTE4:
            // Data is BGRA and upside down, so we need to swizzle to RGB
            for(int j = height-1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + (3*width*(height-1-j));
                unsigned char *src = reinterpret_cast< unsigned char* >( imageData ) + (4*width*j);
                for(int i = 0; i < width; i++) {
                    *dst++ = *(src + 2);
                    *dst++ = *(src + 1);
                    *dst++ = *(src + 0);
                    src += 4;
                }
            }
            break;

        case RT_FORMAT_FLOAT:
            // This buffer is upside down
            for(int j = height-1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + width*(height-1-j);
                float* src = reinterpret_cast< float* >( imageData ) + (3*width*j);
                for(int i = 0; i < width; i++) {
                    int P = static_cast<int>((*src++) * 255.0f);
                    unsigned int Clamped = static_cast< unsigned >( P < 0 ? 0 : P > 0xff ? 0xff : P );

                    // write the pixel to all 3 channels
                    *dst++ = static_cast<unsigned char>(Clamped);
                    *dst++ = static_cast<unsigned char>(Clamped);
                    *dst++ = static_cast<unsigned char>(Clamped);
                }
            }
            break;

        case RT_FORMAT_FLOAT3:
            // This buffer is upside down
            for(int j = height-1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + (3*width*(height-1-j));
                float* src = reinterpret_cast< float* >( imageData ) + (3*width*j);
                for(int i = 0; i < width; i++) {
                    for(int elem = 0; elem < 3; ++elem) {
                        int P = static_cast<int>((*src++) * 255.0f);
                        unsigned int Clamped = static_cast< unsigned >( P < 0 ? 0 : P > 0xff ? 0xff : P );
                        *dst++ = static_cast<unsigned char>(Clamped);
                    }
                }
            }
            break;

        case RT_FORMAT_FLOAT4:
            // This buffer is upside down
            for(int j = height-1; j >= 0; --j) {
                unsigned char *dst = &pix[0] + (3*width*(height-1-j));
                float* src = reinterpret_cast< float*>(imageData) + (4*width*j);
                for(int i = 0; i < width; i++) {
                    for(int elem = 0; elem < 3; ++elem) {
                        int P = static_cast<int>((*src++) * 255.0f);
                        unsigned int Clamped = static_cast< unsigned >( P < 0 ? 0 : P > 0xff ? 0xff : P );
                        *dst++ = static_cast<unsigned char>(Clamped);
                    }

                    // skip alpha
                    src++;
                }
            }
            break;

        default:
            fprintf(stderr, "Unrecognized buffer data type or format.\n");
            exit(2);
            break;
    }

    SavePPM(&pix[0], filename, width, height, 3);

    // Now unmap the buffer
    RT_CHECK_ERROR( rtBufferUnmap(buffer) );
}


void displayBufferPPM( const std::string &filename, optix::Buffer buffer)
{

    displayBufferPPM( filename.c_str(), buffer->get() );

}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


}


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

    context_[ "globalSeed" ]->setUint( dis( gen ) );

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
OptixRenderer::setPathTracing( bool pathTracing )
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


//
// stupid thirdparty code causing warnings
//
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#endif

///
/// \brief OptixRenderer::saveFrame
/// \param filename
///
void
OptixRenderer::saveFrame( const std::string &filename )
{

  displayBufferPPM( filename, getBuffer( ) );

}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


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
