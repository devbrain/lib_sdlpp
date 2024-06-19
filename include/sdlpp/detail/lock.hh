//
// Created by igor on 6/19/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DETAIL_LOCK_HH_
#define SDLPP_INCLUDE_SDLPP_DETAIL_LOCK_HH_

#include <bsw/macros.hh>

namespace neutrino::sdl::detail {
	template <typename T>
	struct locker_traits;

	template <typename T, bool LOCK_IF_NEEDED>
	class locker_impl {
	 public:
		explicit locker_impl (T& s)
			: m_lockable (s),
			  m_locked (true) {
			if constexpr (LOCK_IF_NEEDED) {
				if (locker_traits<T>::must_lock (m_lockable)) {
					locker_traits<T>::lock (m_lockable);
				}
			} else {
				locker_traits<T>::lock (m_lockable);
			}
		}

		~locker_impl() noexcept {
			locker_traits<T>::unlock (m_lockable);
		}

		void release () {
			m_locked = false;
		}

		explicit operator bool () const {
			return m_locked;
		}

	 private:
		T&   m_lockable;
		bool m_locked;
	};

	template<typename T>
	static inline constexpr auto make_lock_impl_always(T& s) {
		return locker_impl<T, false>(s);
	}

	template<typename T>
	static inline constexpr auto make_lock_impl(T& s) {
		return locker_impl<T, true>(s);
	}

}

#define WITH_LOCKED(S)  for(auto ANONYMOUS_VAR(lock_) = ::neutrion::sdl::detail::make_lock_impl(S); ANONYMOUS_VAR(lock_); ANONYMOUS_VAR(lock_).release())
#define WITH_LOCKED_ALWAYS(S)  for(auto ANONYMOUS_VAR(lock_) = ::neutrion::sdl::detail::make_lock_impl_always(S); ANONYMOUS_VAR(lock_); ANONYMOUS_VAR(lock_).release())

#endif //SDLPP_INCLUDE_SDLPP_DETAIL_LOCK_HH_
