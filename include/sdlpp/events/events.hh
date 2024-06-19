//
// Created by igor on 1/10/24.
//

#ifndef SDLPP_EVENTS_HH
#define SDLPP_EVENTS_HH

#include <functional>
#include <algorithm>
#include <vector>
#include <optional>
#include <chrono>
#include <type_traits>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/window_id.hh>
#include <sdlpp/events/events_dispatcher.hh>
#include <bsw/override.hh>

namespace neutrino::sdl {
	template <class...Fs>
	inline bool poll (Fs&& ...fs) {
		static bsw::overload handler (std::forward<Fs> (fs)..., [] (const auto&) {});
		SDL_Event ev;
		if (1 == SDL_PollEvent (&ev)) {
			auto internal_event = map_event (ev);
			std::visit (handler, internal_event);
			return true;
		}
		return false;
	}

	template <class...Fs>
	inline void handle_input (Fs&& ... fs) {
		while (poll (std::forward<Fs> (fs)...)) {}
	}

	SDLPP_EXPORT void add_event_watch (std::function<void (SDL_Event* ev)>& watcher);
	SDLPP_EXPORT void add_event_watch (std::function<void (const events::event_t&)>& watcher);
	SDLPP_EXPORT void remove_event_watch (std::function<void (SDL_Event* ev)>& watcher);
	SDLPP_EXPORT void remove_event_watch (std::function<void (const events::event_t&)>& watcher);

	inline
	bool enable_events_processing (event_type type, bool enable) {
		return SDL_ENABLE == SDL_EventState (static_cast<uint32_t>(type), enable ? SDL_ENABLE : SDL_IGNORE);
	}

	[[nodiscard]] inline
	bool is_event_processing_enabled (event_type type) {
		return SDL_ENABLE == SDL_EventState (static_cast<uint32_t>(type), SDL_QUERY);
	}

	SDLPP_EXPORT void remove_events_if (std::function<bool (SDL_Event* ev)>& predicate);
	SDLPP_EXPORT void remove_events_if (std::function<bool (const events::event_t&)>& predicate);

	SDLPP_EXPORT void set_event_filter (std::function<bool (SDL_Event* ev)>& predicate);
	SDLPP_EXPORT void set_event_filter (std::function<bool (const events::event_t&)>& predicate);
	SDLPP_EXPORT void remove_event_filter();



	inline
	void remove_events (event_type type) {
		SDL_FlushEvent (static_cast<uint32_t>(type));
	}

	inline
	void remove_events (event_type type_from, event_type type_to) {
		SDL_FlushEvents (static_cast<uint32_t>(type_from), static_cast<uint32_t>(type_to));
	}

	[[nodiscard]] inline
	bool has_event (event_type type) {
		return SDL_TRUE == SDL_HasEvent (static_cast<uint32_t>(type));
	}

	[[nodiscard]] inline
	bool has_events (event_type type_from, event_type type_to) {
		return SDL_TRUE == SDL_HasEvents (static_cast<uint32_t>(type_from), static_cast<uint32_t>(type_to));
	}

	template <typename T>
	[[nodiscard]]
	std::vector<SDL_Event*> peek_events (std::size_t num_events, event_type from, event_type to, typename std::enable_if<std::is_same_v<T, SDL_Event*>>::type* = nullptr) {
		std::vector<SDL_Event*> out(num_events, nullptr);
		int rc = SAFE_SDL_CALL(SDL_PeepEvents, out.data(), static_cast<int>(num_events), SDL_PEEKEVENT, static_cast<uint32_t>(from), static_cast<uint32_t>(to));
		out.resize (static_cast<std::size_t>(rc));
		return out;
	}

	template <typename T>
	[[nodiscard]]
	std::vector<SDL_Event*> peek_events (std::size_t num_events, typename std::enable_if<std::is_same_v<T, SDL_Event*>>::type* = nullptr) {
		std::vector<SDL_Event*> out(num_events, nullptr);
		int rc = SAFE_SDL_CALL(SDL_PeepEvents, out.data(), static_cast<int>(num_events), SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
		out.resize (static_cast<std::size_t>(rc));
		return out;
	}

	template <typename T>
	[[nodiscard]]
	std::vector<events::event_t> peek_events (std::size_t num_events, typename std::enable_if<std::is_same_v<T, events::event_t>>::type* = nullptr) {
		std::vector<SDL_Event*> e = peek_events<SDL_Event *>(num_events);
		std::vector<events::event_t> out(e.size());
		std::transform (e.begin(), e.end(), out.begin(), [](const SDL_Event* ev){ return map_event (*ev);});
		return out;
	}

	template <typename T>
	[[nodiscard]]
	std::vector<events::event_t> peek_events (std::size_t num_events, event_type from, event_type to, typename std::enable_if<std::is_same_v<T, events::event_t>>::type* = nullptr) {
		std::vector<SDL_Event*> e = peek_events<SDL_Event *>(num_events, from, to);
		std::vector<events::event_t> out(e.size());
		std::transform (e.begin(), e.end(), out.begin(), [](const SDL_Event* ev){ return map_event (*ev);});
		return out;
	}


	template <typename T>
	[[nodiscard]]
	std::vector<SDL_Event*> remove_events (std::size_t num_events, event_type from, event_type to, typename std::enable_if<std::is_same_v<T, SDL_Event*>>::type* = nullptr) {
		std::vector<SDL_Event*> out(num_events, nullptr);
		int rc = SAFE_SDL_CALL(SDL_PeepEvents, out.data(), static_cast<int>(num_events), SDL_GETEVENT, static_cast<uint32_t>(from), static_cast<uint32_t>(to));
		out.resize (static_cast<std::size_t>(rc));
		return out;
	}

	template <typename T>
	[[nodiscard]]
	std::vector<SDL_Event*> remove_events (std::size_t num_events, typename std::enable_if<std::is_same_v<T, SDL_Event*>>::type* = nullptr) {
		std::vector<SDL_Event*> out(num_events, nullptr);
		int rc = SAFE_SDL_CALL(SDL_PeepEvents, out.data(), static_cast<int>(num_events), SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
		out.resize (static_cast<std::size_t>(rc));
		return out;
	}

	template <typename T>
	[[nodiscard]]
	std::vector<events::event_t> remove_events (std::size_t num_events, typename std::enable_if<std::is_same_v<T, events::event_t>>::type* = nullptr) {
		std::vector<SDL_Event*> e = remove_events<SDL_Event *>(num_events);
		std::vector<events::event_t> out(e.size());
		std::transform (e.begin(), e.end(), out.begin(), [](const SDL_Event* ev){ return map_event (*ev);});
		return out;
	}

	template <typename T>
	[[nodiscard]]
	std::vector<events::event_t> remove_events (std::size_t num_events, event_type from, event_type to, typename std::enable_if<std::is_same_v<T, events::event_t>>::type* = nullptr) {
		std::vector<SDL_Event*> e = remove_events<SDL_Event *>(num_events, from, to);
		std::vector<events::event_t> out(e.size());
		std::transform (e.begin(), e.end(), out.begin(), [](const SDL_Event* ev){ return map_event (*ev);});
		return out;
	}

	inline
	void add_events(const std::vector<SDL_Event>& events) {
		SAFE_SDL_CALL(SDL_PeepEvents, const_cast<std::vector<SDL_Event>&>(events).data(), static_cast<int>(events.size()), SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
	}

	inline
	void pump_events() {
		SDL_PumpEvents();
	}

	inline
	void push_event(const SDL_Event& ev) {
		SAFE_SDL_CALL(SDL_PushEvent, const_cast<SDL_Event*>(&ev));
	}

	inline
	void push_event(const events::user& u) {
		SDL_Event sdl_ev;
		sdl_ev.type = SDL_USEREVENT;
		SDL_UserEvent& ev = sdl_ev.user;
		ev.timestamp = SDL_GetTicks();
		ev.code = u.code;
		ev.data1 = u.data1;
		ev.data2 = u.data2;

		push_event (sdl_ev);
	}

	inline
	void push_event(const events::user& u, window_id_t window_id) {
		SDL_Event sdl_ev;
		sdl_ev.type = SDL_USEREVENT;
		SDL_UserEvent& ev = sdl_ev.user;
		ev.timestamp = SDL_GetTicks();
		ev.windowID = window_id.value_of();
		ev.code = u.code;
		ev.data1 = u.data1;
		ev.data2 = u.data2;

		push_event (sdl_ev);
	}

	inline
	std::optional<events::event_t> wait_for_event() {
		SDL_Event ev;
		if (SDL_WaitEvent (&ev)) {
			return map_event (ev);
		}
		return std::nullopt;
	}

	inline
	std::optional<events::event_t> wait_for_event(std::chrono::milliseconds ms) {
		SDL_Event ev;
		if (SDL_WaitEventTimeout(&ev, static_cast<int>(ms.count()))) {
			return map_event (ev);
		}
		return std::nullopt;
	}
}

#endif //SDLPP_EVENTS_HH
