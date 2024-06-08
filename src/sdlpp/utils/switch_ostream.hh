//
// Created by igor on 6/7/24.
//

#ifndef SDLPP_SRC_SDLPP_UTILS_SWITCH_OSTREAM_HH_
#define SDLPP_SRC_SDLPP_UTILS_SWITCH_OSTREAM_HH_

#include <ostream>
#include <type_traits>
#include <sstream>
#include <bsw/macros.hh>

#define BEGIN_IMPL_OUTPUT(TYPE)                					\
    std::string to_string (TYPE t) {        					\
        switch (t) {

#define END_IMPL_OUTPUT(TYPE)                                	\
        default: return "<UNKNOWN>";                        	\
        }                                                		\
    }                                                        	\
    std::ostream& operator<< (std::ostream& os, TYPE t) {    	\
        os << to_string (t);                                	\
    return os;                                                	\
    }

#define BEGIN_CLASS_OUTPUT(TYPE) 										\
	std::string to_string (const TYPE& t) {                     		\
         std::ostringstream os;                                 		\
         os << t;                                               		\
         return os.str();                  								\
    }                                									\
	std::ostream& operator << (std::ostream& os, const TYPE& t) { 		\
		os << STRINGIZE(TYPE) << '\n';

#define END_CLASS_OUTPUT	return os; }

#define MEMBER(X)	os << "\t" STRINGIZE(X) " : " << t.X << '\n'
#define MEMBER_U8(X)	os << "\t" STRINGIZE(X) " : " << ((int)t.X & 0xFF) << '\n'
#define MEMBER_T(X, T)	os << "\t" STRINGIZE(X) " : " << (T)t.X << '\n'

#endif //SDLPP_SRC_SDLPP_UTILS_SWITCH_OSTREAM_HH_
