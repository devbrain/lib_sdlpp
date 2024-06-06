//
// Created by igor on 6/6/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_IO_RWOPS_HH_
#define SDLPP_INCLUDE_SDLPP_IO_RWOPS_HH_

#include <type_traits>
#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <istream>
#include <ostream>
#include <filesystem>

#include "sdlpp/detail/sdl2.hh"
#include "sdlpp/detail/object.hh"
#include "sdlpp/detail/call.hh"
#include "sdlpp/io/whence.hh"
#include "bsw/mp/introspection.hh"

namespace neutrino::sdl {
	namespace detail::rwops {
		GENERATE_HAS_MEMBER_FUNCTION(close);
		GENERATE_HAS_MEMBER_FUNCTION(seek);
		GENERATE_HAS_MEMBER_FUNCTION(read);
		GENERATE_HAS_MEMBER_FUNCTION(write);
		GENERATE_HAS_MEMBER_FUNCTION(size);
		GENERATE_HAS_MEMBER_STATIC(type_id);
	}

	template <class RWImpl>
	class rwops_base : public object<SDL_RWops> {
	 public:
		rwops_base ()
			: object<SDL_RWops> (_create (), true) {
			handle ()->hidden.unknown.data1 = this;
		}
		// --------------------------------------------------------------------------
	 private:
#define _dENFORCE_TYPE_RWOPS(ret)                                                                           \
    if constexpr (detail::rwops::has_type_id<RWImpl, uint32_t>::value)                                      \
        if (ctx->type != RWImpl::type_id) {                                                                 \
            SDL_SetError("Wrong kind of SDL_RWops. given : %d , expected %d", ctx->type, RWImpl::type_id);  \
            return ret;                                                                                     \
        } ((void)0)

		static SDL_RWops* _create () {
			auto* ret = SDL_AllocRW ();

			if constexpr (detail::rwops::has_type_id<RWImpl, uint32_t>::value) {
				static_assert (!(
								   (RWImpl::type_id == SDL_RWOPS_WINFILE) ||
								   (RWImpl::type_id == SDL_RWOPS_STDFILE) ||
								   (RWImpl::type_id == SDL_RWOPS_JNIFILE) ||
								   (RWImpl::type_id == SDL_RWOPS_MEMORY) ||
								   (RWImpl::type_id == SDL_RWOPS_MEMORY_RO)),
							   "Predefined RWOps type is used");

				ret->type = RWImpl::type_id;
			} else {
				ret->type = SDL_RWOPS_UNKNOWN;
			}

			if constexpr (detail::rwops::has_size<RWImpl, int64_t ()>::value) {
				ret->size = [] (SDL_RWops* ctx) -> int64_t {
				  _dENFORCE_TYPE_RWOPS(-1);
				  return reinterpret_cast<RWImpl*>(ctx->hidden.unknown.data1)->size ();
				};
			} else {
				/* size is a function pointer that reports the stream's total size in bytes.
				 * If the stream size can't be determined (either because it doesn't make sense for the stream type, or there was an error),
				 * this function returns -1.
				 */
				ret->size = [] ([[maybe_unused]] SDL_RWops* ctx) -> int64_t {
				  SDL_SetError ("Method size is not implemented");
				  return -1;
				};
			}

			if constexpr (detail::rwops::has_seek<RWImpl, int64_t (int64_t, whence)>::value) {
				ret->seek = [] (SDL_RWops* ctx, Sint64 v, int o) -> int64_t {
				  _dENFORCE_TYPE_RWOPS(-1);
				  return reinterpret_cast<RWImpl*>(ctx->hidden.unknown.data1)->seek (v, static_cast<whence>(o));
				};
			} else {
				ret->seek = [] (SDL_RWops*, Sint64, int) -> int64_t {
				  SDL_SetError ("Method seek is not implemented");
				  return -1;
				};
			}
			if constexpr (detail::rwops::has_read<RWImpl, size_t (void*, size_t, size_t)>::value) {
				ret->read = [] (SDL_RWops* ctx, void* buff, size_t s, size_t n) -> size_t {
				  _dENFORCE_TYPE_RWOPS(0);
				  return reinterpret_cast<RWImpl*>(ctx->hidden.unknown.data1)->read (buff, s, n);
				};
			} else {
				ret->read = [] ([[maybe_unused]] SDL_RWops* ctx, [[maybe_unused]] void* buff, [[maybe_unused]] size_t s,
								[[maybe_unused]] size_t n) -> size_t {
				  SDL_SetError ("Method read is not implemented");
				  return 0;
				};
			}
			// std::size_t write (const void* ptr, std::size_t size, std::size_t maxnum)
			if constexpr (detail::rwops::has_write<RWImpl, size_t (const void*, size_t, size_t)>::value) {
				ret->write = [] (SDL_RWops* ctx, const void* buff, size_t s, size_t n) -> size_t {
				  _dENFORCE_TYPE_RWOPS(0);
				  return reinterpret_cast<RWImpl*>(ctx->hidden.unknown.data1)->write (buff, s, n);
				};
			} else {
				ret->write = [] ([[maybe_unused]] SDL_RWops* ctx, [[maybe_unused]] const void* buff,
								 [[maybe_unused]] size_t s, [[maybe_unused]] size_t n) -> size_t {
				  SDL_SetError ("Method write is not implemented");
				  return 0;
				};
			}

			ret->close = [] (SDL_RWops* ctx) -> int {
			  _dENFORCE_TYPE_RWOPS(-1);
			  //  delete reinterpret_cast<RWImpl*>(ctx->hidden.unknown.data1);

			  return 0;
			};
			return ret;
		}
	};
}

#endif //SDLPP_INCLUDE_SDLPP_IO_RWOPS_HH_
