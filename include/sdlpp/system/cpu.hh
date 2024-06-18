//
// Created by igor on 6/6/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_CPU_HH_
#define SDLPP_INCLUDE_SDLPP_CPU_HH_

#include <cstddef>
#include <array>
#include <type_traits>

#include <bitflags/bitflags.hpp>
#include <bsw/macros.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	class cpu {
	 public:
		BEGIN_BITFLAGS(capability)
			FLAG(HAS_3DNow)
			FLAG(HAS_AltiVec)
			FLAG(HAS_ARMSIMD)
			FLAG(HAS_AVX)
			FLAG(HAS_AVX2)
			FLAG(HAS_AVX512F)
			FLAG(HAS_LASX)
			FLAG(HAS_LSX)
			FLAG(HAS_MMX)
			FLAG(HAS_NEON)
			FLAG(HAS_RDTSC)
			FLAG(HAS_SSE)
			FLAG(HAS_SSE2)
			FLAG(HAS_SSE3)
			FLAG(HAS_SSE41)
			FLAG(HAS_SSE42)
		END_BITFLAGS(capability)

		// in bytes
		static unsigned int cache_line() {
			return static_cast<unsigned int>(SDL_GetCPUCacheLineSize());
		}
		static unsigned int count() {
			return static_cast<unsigned int>(SDL_GetCPUCount());
		}

		static capability capabilities() {
			static auto c = eval_cap();
			return c;
		}

		static void* simd_alloc(std::size_t len) {
			return SDL_SIMDAlloc (len);
		}

		static void* simd_realloc(void* p, std::size_t len) {
			return SDL_SIMDRealloc(p, len);
		}

		static void simd_free(void* p) {
			SDL_SIMDFree (p);
		}

		static std::size_t simd_alignment() {
			return SDL_SIMDGetAlignment();
		}


	 private:
#define d_evalCap(NAME) 									\
		if (PPCAT(SDL_Has, NAME)()) { 						\
	 		c |= capability::PPCAT(HAS_, NAME);           	\
		}

		static capability eval_cap() {
			capability c{};
			d_evalCap(3DNow)
			d_evalCap(AltiVec)
			d_evalCap(ARMSIMD)
			d_evalCap(AVX)
			d_evalCap(AVX2)
			d_evalCap(AVX512F)
			d_evalCap(LASX)
			d_evalCap(LSX)
			d_evalCap(MMX)
			d_evalCap(NEON)
			d_evalCap(RDTSC)
			d_evalCap(SSE)
			d_evalCap(SSE2)
			d_evalCap(SSE3)
			d_evalCap(SSE41)
			d_evalCap(SSE42)
			return c;
		}

#undef d_evalCap
	};



	namespace detail {
		static inline constexpr std::array<cpu::capability::flag_type, 16> s_vals_of_cpu_capability = {
			cpu::capability::HAS_3DNow,
			cpu::capability::HAS_AltiVec,
			cpu::capability::HAS_ARMSIMD,
			cpu::capability::HAS_AVX,
			cpu::capability::HAS_AVX2,
			cpu::capability::HAS_AVX512F,
			cpu::capability::HAS_LASX,
			cpu::capability::HAS_LSX,
			cpu::capability::HAS_MMX,
			cpu::capability::HAS_NEON,
			cpu::capability::HAS_RDTSC,
			cpu::capability::HAS_SSE,
			cpu::capability::HAS_SSE2,
			cpu::capability::HAS_SSE3,
			cpu::capability::HAS_SSE41,
			cpu::capability::HAS_SSE42,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_cpu_capability)&
	values(typename std::enable_if<std::is_same_v<cpu::capability, T>>::type* = nullptr) {
		return detail::s_vals_of_cpu_capability;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<cpu::capability, T>>::type* = nullptr) {
		return detail::s_vals_of_cpu_capability.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<cpu::capability, T>>::type* = nullptr) {
		return detail::s_vals_of_cpu_capability.end();
	}

	d_SDLPP_OSTREAM_WITHOT_FROM_STRING(cpu::capability);
}

#endif //SDLPP_INCLUDE_SDLPP_CPU_HH_
