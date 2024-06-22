//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_SHARED_OBJECT_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_SHARED_OBJECT_HH_

#include <memory>
#include <filesystem>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <bsw/exception.hh>

namespace neutrino::sdl {
	class shared_object {
		public:
			explicit shared_object(const char* path)
				: m_object(_load(path)) {
			}

			explicit shared_object(const std::string& path)
				: m_object(_load(path.c_str())) {
			}

			explicit shared_object(const std::filesystem::path& path)
				: m_object(_load(path.u8string().c_str())) {
			}

			template<typename T>
			[[nodiscard]] T* load_function(const char* name) const {
				ENFORCE(name != nullptr);
				return static_cast <T*>(SDL_LoadFunction(m_object.get(), name));
			}

			template<typename T>
			[[nodiscard]] T load_function(const std::string& name) const {
				return load_function <T>(name.c_str());
			}

		private:
			static void* _load(const char* path) {
				ENFORCE(path != nullptr);
				void* handle = SAFE_SDL_CALL(SDL_LoadObject, path);
				return handle;
			}

			struct object_deleter final {
				void operator()(void* object) const noexcept { SDL_UnloadObject(object); }
			};

			std::unique_ptr <void, object_deleter> m_object;
	};
}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_SHARED_OBJECT_HH_
