//
// Created by igor on 7/24/25.
//

#pragma once

/**
 * @file euler.hh
 * @brief Central include for euler library with warning suppression
 *
 * This header wraps euler library includes to suppress warnings from
 * third-party code (euler uses xsimd which triggers various warnings
 * with strict compiler flags).
 */

#include <sdlpp/detail/compiler.hh>

// Suppress warnings from euler/xsimd headers
#if defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wundef"
# pragma GCC diagnostic ignored "-Wold-style-cast"
# pragma GCC diagnostic ignored "-Wsign-conversion"
# pragma GCC diagnostic ignored "-Wdouble-promotion"
# pragma GCC diagnostic ignored "-Wcast-align"
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wundef"
# pragma clang diagnostic ignored "-Wold-style-cast"
# pragma clang diagnostic ignored "-Wsign-conversion"
# pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
# pragma clang diagnostic ignored "-Wdouble-promotion"
# pragma clang diagnostic ignored "-Wcast-align"
#elif defined(SDLPP_COMPILER_MSVC)
# pragma warning(push)
# pragma warning(disable: 4244)  // conversion, possible loss of data
# pragma warning(disable: 4267)  // conversion from size_t
#endif

// DDA iterators
#include <euler/dda/line_iterator.hh>
#include <euler/dda/aa_line_iterator.hh>
#include <euler/dda/thick_line_iterator.hh>
#include <euler/dda/circle_iterator.hh>
#include <euler/dda/ellipse_iterator.hh>
#include <euler/dda/arc_iterator.hh>
#include <euler/dda/bezier_iterator.hh>
#include <euler/dda/batched_line_iterator.hh>
#include <euler/dda/batched_bezier_iterator.hh>
#include <euler/dda/bspline_iterator.hh>
#include <euler/dda/pixel_batch.hh>

// Angles
#include <euler/angles/angle.hh>
#include <euler/angles/radian.hh>

#if defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic pop
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic pop
#elif defined(SDLPP_COMPILER_MSVC)
# pragma warning(pop)
#endif
