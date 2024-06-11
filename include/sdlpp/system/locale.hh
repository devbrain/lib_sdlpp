//
// Created by igor on 6/11/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_LOCALE_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_LOCALE_HH_

#include <memory>
#include <cstring>
#include <string>
#include <sdlpp/detail/sdl2.hh>
#include <bsw/exception.hh>

namespace neutrino::sdl {
	class locale {
	 public:
		[[nodiscard]] static locale get_preferred () noexcept {
			return locale{SDL_GetPreferredLocales ()};
		}

		[[nodiscard]] bool has_language (const char* language,
										 const char* country = nullptr) const noexcept {
			ENFORCE(language != nullptr);
			if (const auto* array = m_locale.get ()) {
				for (auto index = 0u; array[index].language; ++index) {
					const auto& item = array[index];

					if (country && item.country) {
						if (_cmp (language, item.language) && _cmp (country, item.country)) {
							return true;
						}
					} else {
						if (_cmp (language, item.language)) {
							return true;
						}
					}
				}
			}

			return false;
		}

		[[nodiscard]] bool has_language (const std::string& language,
										 const std::string& country = {}) const noexcept {
			if (country.empty ()) {
				return has_language (language.c_str (), nullptr);
			}
			return has_language (language.c_str (), country.c_str ());
		}

		explicit operator bool() const noexcept { return m_locale != nullptr; }
	 private:
		explicit locale (SDL_Locale* loc)
			: m_locale (loc) {}

	 private:

		static bool _cmp (const char* a, const char* b) {
			if (a && b) {
				return std::strcmp (a, b) == 0;
			} else {
				return false;
			}
		}

		struct sdl_locale_deleter {
			void operator() (void* ptr) noexcept { SDL_free (ptr); }
		};

		std::unique_ptr<SDL_Locale, sdl_locale_deleter> m_locale;
	};
}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_LOCALE_HH_
