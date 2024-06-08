//
// Created by igor on 6/8/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_OSTREAMOPS_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_OSTREAMOPS_HH_

#include <string>
#include <iosfwd>
#include <sdlpp/export_defines.h>

#define d_SDLPP_OSTREAM(TYPE) 											\
	SDLPP_EXPORT std::string to_string(TYPE t);							\
	SDLPP_EXPORT std::ostream& operator << (std::ostream& os, TYPE t)
#endif //SDLPP_INCLUDE_SDLPP_DETAIL_OSTREAMOPS_HH_
