#pragma once

// =============================================================================
// Collision shapes — pure world space (umbrella header).
//
// This module knows nothing about display, dp, scale, or SDL. Everything here
// operates on a plain numeric field (float world units) so the collision math
// can use products of lengths freely (dot, cross, squared distance, SAT
// projections) and can be unit-tested with no window or global scale state.
//
// Conversions to/from display-space (simplex::point / rect / circle, which are
// dp-based) live in <simplex/collide/bridge.hh> and are the *only* place that
// bridges the two coordinate systems.
//
// This header is an umbrella that re-exports the whole module; include it for
// the full API, or include a part directly:
//   - <simplex/collide/types.hh>     value types + result types
//   - <simplex/collide/distance.hh>  closest-point / squared-distance queries
//   - <simplex/collide/overlap.hh>   static boolean overlap predicates
//   - <simplex/collide/sweep.hh>     parametric + continuous collision queries
//
// Query categories (kept distinct in both naming and result type):
//   - intersects(A, B)            -> bool       : static overlap of two shapes.
//   - intersect_param(shape, seg) -> line_hit   : raw geometry; entry/exit are
//                                                 PARAMETERS along the segment line.
//   - swept_intersection(A,av,B,bv,time)
//                                 -> swept_hit  : continuous collision; entry/exit
//                                                 are absolute TIMES in seconds.
//   - overlap(A, B)               -> penetration: static MTV to separate A from B.
//
// Supported shape pairs (argument order is interchangeable where both forms exist):
//   contains:            (aabb|circle|segment, point)          -- full point row of the matrix
//   intersects (static): every pair of {point,segment,circle,aabb} (point via contains)
//   overlap (MTV):       (aabb,aabb) (circle,circle) (circle,aabb)|(aabb,circle)  -> penetration
//   intersect_param:     (aabb,segment) (circle,segment) (segment,segment)   -> line_hit
//   swept_intersection:  (aabb,aabb) (circle,circle) (circle,aabb)|(aabb,circle) -> swept_hit
//   closest_point:       (point, segment|aabb|circle)
//   squared_distance:    every pair of {point,segment,circle,aabb} (filled regions; 0 if overlapping)
//
// constexpr: a query is constexpr exactly when it avoids std::sqrt, which is not
// constexpr in C++20 (euler's vector/matrix expression evaluation is otherwise
// constexpr). That covers the value-type accessors, contains, every static overlap
// predicate, intersect_param(aabb,segment), closest_point / closest_parameter /
// squared_distance for point-vs-segment and point-vs-aabb, swept_intersection(aabb,aabb),
// and to_swept_hit. Runtime-only are the paths that take a square root: anything against
// a circle radius (squared_distance/closest_point vs circle, intersect_param(circle,
// segment), the circle CCD overloads) and segment/segment (its normals are normalized).
// =============================================================================

#include <simplex/collide/types.hh>
#include <simplex/collide/distance.hh>
#include <simplex/collide/overlap.hh>
#include <simplex/collide/sweep.hh>
