//
// Created by igor on 6/19/24.
//
#include <sdlpp/events/events.hh>

static int SDLCALL event_watch_simple (void *userdata, SDL_Event* event) {
	auto* f = reinterpret_cast<std::function<void (SDL_Event* ev)>*>(userdata);
	(*f)(event);
	return 0;
}

static int SDLCALL event_watch_complex (void *userdata, SDL_Event* event) {
	if (event) {
		auto* f = reinterpret_cast<std::function<void (const neutrino::sdl::events::event_t&)>*>(userdata);
		auto internal_event = neutrino::sdl::map_event (*event);
		(*f) (internal_event);
	}
	return 0;
}

static int SDLCALL event_remove_simple (void *userdata, SDL_Event* event) {
	auto* f = reinterpret_cast<std::function<bool (SDL_Event* ev)>*>(userdata);
	if ((*f)(event)) {
		return 0;
	}
	return 1;
}

static int SDLCALL event_remove_complex (void *userdata, SDL_Event* event) {
	if (event) {
		auto* f = reinterpret_cast<std::function<bool (const neutrino::sdl::events::event_t&)>*>(userdata);
		auto internal_event = neutrino::sdl::map_event (*event);
		if ((*f) (internal_event)) {
			return 0;
		}
		return 1;
	}
	return 0;
}

static int SDLCALL event_filter_simple (void *userdata, SDL_Event* event) {
	auto* f = reinterpret_cast<std::function<bool (SDL_Event* ev)>*>(userdata);
	if ((*f)(event)) {
		return 1;
	}
	return 0;
}

static int SDLCALL event_filter_complex (void *userdata, SDL_Event* event) {
	if (event) {
		auto* f = reinterpret_cast<std::function<bool (const neutrino::sdl::events::event_t&)>*>(userdata);
		auto internal_event = neutrino::sdl::map_event (*event);
		if ((*f) (internal_event)) {
			return 1;
		}
		return 0;
	}
	return 1;
}

static int SDLCALL nop_filter (void *userdata, SDL_Event* event) {
	return 1;
}

namespace neutrino::sdl{
	void add_event_watch(std::function<void (SDL_Event* ev)>& watcher) {
		SDL_AddEventWatch (event_watch_simple, &watcher);
	}

	void add_event_watch(std::function<void (const events::event_t&)>& watcher) {
		SDL_AddEventWatch (event_watch_complex, &watcher);
	}

	void remove_event_watch(std::function<void (SDL_Event* ev)>& watcher) {
		SDL_DelEventWatch (event_watch_simple, &watcher);
	}

	void remove_event_watch(std::function<void (const events::event_t&)>& watcher) {
		SDL_DelEventWatch (event_watch_complex, &watcher);
	}

	void remove_events_if (std::function<bool (SDL_Event* ev)>& predicate) {
		SDL_FilterEvents (event_remove_simple, &predicate);
	}

	void remove_events_if (std::function<bool (const events::event_t&)>& predicate) {
		SDL_FilterEvents (event_remove_complex, &predicate);
	}

	void set_event_filter (std::function<bool (SDL_Event* ev)>& predicate) {
		SDL_SetEventFilter(event_filter_simple, &predicate);
	}

	void set_event_filter (std::function<bool (const events::event_t&)>& predicate) {
		SDL_SetEventFilter (event_filter_complex, &predicate);
	}

	void remove_event_filter() {
		SDL_SetEventFilter (nop_filter, nullptr);
	}
}