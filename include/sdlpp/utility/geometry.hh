//
// SDL++ Geometry - Conditional Inclusion Header
// Copyright (C) 2024 SDL++
//
// This header includes geometry concepts and optionally includes concrete types
//

#ifndef SDLPP_UTILITY_GEOMETRY_HH
#define SDLPP_UTILITY_GEOMETRY_HH

// Always include concepts - they have zero dependencies
#include <sdlpp/utility/geometry_concepts.hh>

// Conditionally include concrete types
#ifndef SDLPP_NO_BUILTIN_GEOMETRY
    #include <sdlpp/utility/geometry_types.hh>
#endif

// Feature detection macros
#ifdef SDLPP_NO_BUILTIN_GEOMETRY
    #define SDLPP_HAS_BUILTIN_GEOMETRY 0
#else
    #define SDLPP_HAS_BUILTIN_GEOMETRY 1
#endif

// Additional geometry algorithms that work with any geometry types
#include <sdlpp/utility/geometry_algorithms.hh>

#endif // SDLPP_UTILITY_GEOMETRY_HH