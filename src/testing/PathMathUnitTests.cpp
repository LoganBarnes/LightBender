#include <vector>
#include <cmath>
#include "gmock/gmock.h"
#include <optixu/optixpp_namespace.h>
#include "optixMod/optix_math_stream_namespace_mod.h"
#include "random.h"


namespace
{


class PathMathUnitTests : public ::testing::Test
{

protected:

  ///
  /// \brief buildExpectedStaticVector
  /// \param screen
  /// \param sqrt_num_samples
  /// \return
  ///
  static
  std::vector< optix::float2 >
  buildExpectedStaticVector(
                            optix::uint2 screen,
                            unsigned     sqrt_num_samples
                            )
  {

    std::vector< optix::float2 > result;

    int totalRows = static_cast< int >( screen.y * sqrt_num_samples );
    int totalCols = static_cast< int >( screen.x * sqrt_num_samples );

    for ( int y = -totalRows + 1; y < totalRows; y += 2 )
    {

      float yf = y * 1.0f / totalRows;

      for ( int x = -totalCols + 1; x < totalCols; x += 2 )
      {

        float xf = x * 1.0f / totalCols;
        result.push_back( optix::make_float2( xf, yf ) );

      }

    }

    return result;

  } // buildExpectedStaticVector


  ///
  /// \brief buildExpectedLowerBound
  /// \param screen
  /// \param sqrt_num_samples
  /// \return
  ///
  static
  std::vector< optix::float2 >
  buildExpectedLowerBound(
                          optix::uint2 screen,
                          unsigned     sqrt_num_samples
                          )
  {

    std::vector< optix::float2 > result;

    int totalRows = static_cast< int >( screen.y * sqrt_num_samples );
    int totalCols = static_cast< int >( screen.x * sqrt_num_samples );

    for ( int y = -totalRows; y < totalRows - 1; y += 2 )
    {

      float yf = y * 1.0f / totalRows;

      for ( int x = -totalCols; x < totalCols - 1; x += 2 )
      {

        float xf = x * 1.0f / totalCols;
        result.push_back( optix::make_float2( xf, yf ) );

      }

    }

    return result;

  } // buildExpectedLowerBound


  ///
  /// \brief buildExpectedUpperBound
  /// \param screen
  /// \param sqrt_num_samples
  /// \return
  ///
  static
  std::vector< optix::float2 >
  buildExpectedUpperBound(
                          optix::uint2 screen,
                          unsigned     sqrt_num_samples
                          )
  {

    std::vector< optix::float2 > result;

    int totalRows = static_cast< int >( screen.y * sqrt_num_samples );
    int totalCols = static_cast< int >( screen.x * sqrt_num_samples );

    for ( int y = -totalRows + 2; y <= totalRows; y += 2 )
    {

      float yf = y * 1.0f / totalRows;

      for ( int x = -totalCols + 2; x <= totalCols; x += 2 )
      {

        float xf = x * 1.0f / totalCols;
        result.push_back( optix::make_float2( xf, yf ) );

      }

    }

    return result;

  } // buildExpectedUpperBound


  ///
  /// \brief buildFullPixelScreenSpaceSamples
  /// \param screen
  /// \param sqrt_num_samples
  /// \param staticSamples
  /// \return
  ///
  static
  std::vector< optix::float2 >
  buildFullPixelScreenSpaceSamples(
                                   optix::uint2 screen,
                                   unsigned     sqrt_num_samples,
                                   bool         staticSamples = true
                                   )
  {

    std::vector< optix::float2 > samples;
    std::vector< optix::float2 > pixSamples;

    optix::uint2 launch_index;

    for ( unsigned y = 0; y < screen.y; ++y )
    {

      launch_index.y = y;

      for ( unsigned x = 0; x < screen.x; ++x )
      {

        launch_index.x = x;

        pixSamples = buildSinglePixelScreenSpaceSamples(
                                                        screen,
                                                        launch_index,
                                                        sqrt_num_samples,
                                                        staticSamples
                                                        );

        samples.insert(
                       std::end  ( samples ),
                       std::begin( pixSamples ),
                       std::end  ( pixSamples )
                       );

      }

    }

    return samples;

  } // buildFullPixelScreenSpaceSamples


  ///
  /// \brief buildSinglePixelScreenSpaceSamples
  /// \param screen
  /// \param launch_index
  /// \param sqrt_num_samples
  /// \param staticSamples
  /// \return
  ///
  static
  std::vector< optix::float2 >
  buildSinglePixelScreenSpaceSamples(
                                     optix::uint2 screen,
                                     optix::uint2 launch_index,
                                     unsigned     sqrt_num_samples,
                                     bool         staticSamples = true
                                     )
  {

    optix::float2 inv_screen  = 1.0f / optix::make_float2( screen ) * 2.0f;
    optix::float2 pixelCorner = optix::make_float2( launch_index ) * inv_screen - 1.0f;

    optix::float2 jitter_scale     = inv_screen / static_cast< float >( sqrt_num_samples );
    unsigned int samples_per_pixel = sqrt_num_samples * sqrt_num_samples;

    std::vector< optix::float2 > samples( samples_per_pixel );

    unsigned seed = 0;

    if ( !staticSamples )
    {

      seed = tea< 16 >( screen.x * launch_index.y + launch_index.x, 1 );

    }

    // loop vars
    unsigned x, y;
    optix::float2 jitter;

    // faster two for loops for x and y on gpu?
    while ( samples_per_pixel-- )
    {

      x = samples_per_pixel % sqrt_num_samples;
      y = samples_per_pixel / sqrt_num_samples;

      jitter = optix::make_float2(
                                  static_cast< float >( x ),
                                  static_cast< float >( y )
                                  );

      if ( staticSamples )
      {

        // no random jitter, just center value
        jitter += optix::make_float2( 0.5f );

      }
      else
      {

        jitter += optix::make_float2( rnd( seed ), rnd( seed ) );

      }

      samples[ samples_per_pixel ] = pixelCorner + jitter * jitter_scale;

    }

    return samples;

  } // buildSinglePixelScreenSpaceSamples


};



//////////////////////////////////////////////////////////
// compares float2s
////////////////////////////////////////////////////////
MATCHER( Float2Near, "is equal within 1.0e-7" )
{

  constexpr float eps = 1.0e-7f;

  optix::float2 a    = ::testing::get< 0 >( arg );
  optix::float2 b    = ::testing::get< 1 >( arg );
  optix::float2 diff = a - b;
  return std::abs( diff.x ) < eps && std::abs( diff.y ) < eps;

}


//////////////////////////////////////////////////////////
// compares float2s
////////////////////////////////////////////////////////
MATCHER( Float2Le, "is less than or equal to" )
{

  optix::float2 a = ::testing::get< 0 >( arg );
  optix::float2 b = ::testing::get< 1 >( arg );
  return a.x <= b.x && a.y <= b.y;

}



//////////////////////////////////////////////////////////
// compares float2s
////////////////////////////////////////////////////////
MATCHER( Float2Ge, "is less than or equal to" )
{

  optix::float2 a = ::testing::get< 0 >( arg );
  optix::float2 b = ::testing::get< 1 >( arg );
  return a.x >= b.x && a.y >= b.y;

}



////////////////////////////////////////////////////////
// test pointwise container comparison
////////////////////////////////////////////////////////
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"

TEST_F( PathMathUnitTests, PointwiseMatcherHelpers )
{

#pragma GCC diagnostic pop

  using namespace ::testing;

  constexpr unsigned numElmts = 100;

  std::vector< optix::float2 > mid  ( numElmts );
  std::vector< optix::float2 > lower( numElmts );
  std::vector< optix::float2 > upper( numElmts );

  optix::float2 f2;

  unsigned seed = tea< 16 >( 0, 1 );


  for ( unsigned i = 0; i < numElmts; ++i )
  {

    f2.x = rnd( seed );
    f2.y = rnd( seed );

    mid  [ i ] = f2;
    lower[ i ] = f2 - optix::make_float2( 0.5 );
    upper[ i ] = f2 + optix::make_float2( 0.5 );

  }

  ASSERT_THAT( mid, Pointwise( Float2Le( ), upper ) );
  ASSERT_THAT( mid, Pointwise( Float2Ge( ), lower ) );

}



//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"

TEST_F( PathMathUnitTests, VectorBuilderHelpers )
{

#pragma GCC diagnostic pop

  using namespace ::testing;

  //
  // 3x2 screen w/ 1 sample each
  //
  optix::uint2 screenSize = optix::make_uint2( 4, 2 );
  unsigned sqrt_samples   = 1;

  std::vector< optix::float2 > expected =
  {

    optix::make_float2( -0.75f, -0.5f ),
    optix::make_float2( -0.25f, -0.5f ),
    optix::make_float2( 0.25f,  -0.5f ),
    optix::make_float2( 0.75f,  -0.5f ),

    optix::make_float2( -0.75f, 0.5f ),
    optix::make_float2( -0.25f, 0.5f ),
    optix::make_float2( 0.25f,  0.5f ),
    optix::make_float2( 0.75f,  0.5f )

  };


  std::vector< optix::float2 > lower =
  {

    optix::make_float2( -1.0f, -1.0f ),
    optix::make_float2( -0.5f, -1.0f ),
    optix::make_float2( 0.0f,  -1.0f ),
    optix::make_float2( 0.5f,  -1.0f ),

    optix::make_float2( -1.0f, 0.0f ),
    optix::make_float2( -0.5f, 0.0f ),
    optix::make_float2( 0.0f,  0.0f ),
    optix::make_float2( 0.5f,  0.0f )

  };


  std::vector< optix::float2 > upper =
  {

    optix::make_float2( -0.5f, 0.0f ),
    optix::make_float2( 0.0f,  0.0f ),
    optix::make_float2( 0.5f,  0.0f ),
    optix::make_float2( 1.0f,  0.0f ),

    optix::make_float2( -0.5f, 1.0f ),
    optix::make_float2( 0.0f,  1.0f ),
    optix::make_float2( 0.5f,  1.0f ),
    optix::make_float2( 1.0f,  1.0f )

  };


  std::vector< optix::float2 > computed =
    PathMathUnitTests::buildExpectedStaticVector(
                                                 screenSize,
                                                 sqrt_samples
                                                 );


  ASSERT_THAT( computed, Pointwise( Float2Near( ), expected ) );



  computed = PathMathUnitTests::buildExpectedLowerBound(
                                                        screenSize,
                                                        sqrt_samples
                                                        );

  ASSERT_THAT( computed, Pointwise( Float2Near( ), lower ) );



  computed = PathMathUnitTests::buildExpectedUpperBound(
                                                        screenSize,
                                                        sqrt_samples
                                                        );


  ASSERT_THAT( computed, Pointwise( Float2Near( ), upper ) );


}



//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"

TEST_F( PathMathUnitTests, NoJitterSingleSample )
{

#pragma GCC diagnostic pop

  using namespace ::testing;

  std::vector< optix::float2 > expected, computed;


  //
  // 3x2 screen w/ 1 sample each
  //
  optix::uint2 screenSize = optix::make_uint2( 3, 2 );
  unsigned sqrt_samples   = 1;

  expected = PathMathUnitTests::buildExpectedStaticVector(
                                                          screenSize,
                                                          sqrt_samples
                                                          );

  computed = PathMathUnitTests::buildFullPixelScreenSpaceSamples(
                                                                 screenSize,
                                                                 sqrt_samples
                                                                 );

  ASSERT_THAT( computed, Pointwise( Float2Near( ), expected ) );


  //
  // 4x2 screen w/ 2 samples each
  //
  screenSize   = optix::make_uint2( 4, 2 );
  sqrt_samples = 2;

  optix::uint2 launch_index = optix::make_uint2( 0, 0 );

  expected =
  {

    optix::make_float2( -7.0f / 8.0f, -0.75f ),
    optix::make_float2( -5.0f / 8.0f, -0.75f ),
    optix::make_float2( -7.0f / 8.0f, -0.25f ),
    optix::make_float2( -5.0f / 8.0f, -0.25f )

  };

  computed = PathMathUnitTests::buildSinglePixelScreenSpaceSamples(
                                                                   screenSize,
                                                                   launch_index,
                                                                   sqrt_samples
                                                                   );

  ASSERT_THAT( computed, Pointwise( Float2Near( ), expected ) );


  // NOT BUILDING MULTISAMPLE VECS CORRECTLY YET
//  expected = PathMathUnitTests::buildExpectedStaticVector(
//                                                          screenSize,
//                                                          sqrt_samples
//                                                          );

//  computed = PathMathUnitTests::buildFullPixelScreenSpaceSamples(
//                                                                 screenSize,
//                                                                 sqrt_samples
//                                                                 );

//  ASSERT_THAT( computed, Pointwise( Float2Near( ), expected ) );

}



//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-overflow"

TEST_F( PathMathUnitTests, JitterSingleSample )
{

#pragma GCC diagnostic pop

  using namespace ::testing;

  std::vector< optix::float2 > lower, upper, computed;


  //
  // 3x2 screen w/ 1 sample each
  //
  optix::uint2 screenSize = optix::make_uint2( 3, 2 );
  unsigned sqrt_samples   = 1;

  lower = PathMathUnitTests::buildExpectedLowerBound(
                                                     screenSize,
                                                     sqrt_samples
                                                     );

  upper = PathMathUnitTests::buildExpectedUpperBound(
                                                     screenSize,
                                                     sqrt_samples
                                                     );

  computed = PathMathUnitTests::buildFullPixelScreenSpaceSamples(
                                                                 screenSize,
                                                                 sqrt_samples,
                                                                 true
                                                                 );

  ASSERT_THAT( computed, Pointwise( Float2Le( ), upper ) );
  ASSERT_THAT( computed, Pointwise( Float2Ge( ), lower ) );


  //
  // 4x2 screen w/ 2 samples each
  //
  screenSize   = optix::make_uint2( 4, 2 );
  sqrt_samples = 2;

  optix::uint2 launch_index = optix::make_uint2( 0, 0 );

  lower =
  {

    optix::make_float2( -8.0f / 8.0f, -1.0f ),
    optix::make_float2( -6.0f / 8.0f, -1.0f ),
    optix::make_float2( -8.0f / 8.0f, -0.5f ),
    optix::make_float2( -6.0f / 8.0f, -0.5f )

  };

  upper =
  {

    optix::make_float2( -6.0f / 8.0f, -0.5f ),
    optix::make_float2( -4.0f / 8.0f, -0.5f ),
    optix::make_float2( -6.0f / 8.0f,  0.0f ),
    optix::make_float2( -4.0f / 8.0f,  0.0f )

  };

  computed = PathMathUnitTests::buildSinglePixelScreenSpaceSamples(
                                                                   screenSize,
                                                                   launch_index,
                                                                   sqrt_samples,
                                                                   true
                                                                   );

  ASSERT_THAT( computed, Pointwise( Float2Le( ), upper ) );
  ASSERT_THAT( computed, Pointwise( Float2Ge( ), lower ) );


  // NOT BUILDING MULTISAMPLE VECS CORRECTLY YET
//  lower = PathMathUnitTests::buildExpectedLowerBound(
//                                                     screenSize,
//                                                     sqrt_samples
//                                                     );

//  upper = PathMathUnitTests::buildExpectedUpperBound(
//                                                     screenSize,
//                                                     sqrt_samples
//                                                     );

//  computed = PathMathUnitTests::buildFullPixelScreenSpaceSamples(
//                                                                 screenSize,
//                                                                 sqrt_samples
//                                                                 );

//  ASSERT_THAT( computed, Pointwise( Float2Le( ), upper ) );
//  ASSERT_THAT( computed, Pointwise( Float2Ge( ), lower ) );

}



} // namespace
