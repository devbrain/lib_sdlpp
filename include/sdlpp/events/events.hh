//
// Created by igor on 1/10/24.
//

#ifndef SDLPP_EVENTS_HH
#define SDLPP_EVENTS_HH

#include <sdlpp/events/events_dispatcher.hh>
#include <bsw/override.hh>


namespace neutrino::sdl {
    template <class...Fs>
    inline bool poll(Fs&&...fs) {
        static bsw::overload handler(std::forward<Fs>(fs)..., [](const auto&) {});
        SDL_Event ev;
        if (1 == SDL_PollEvent(&ev)) {
            auto internal_event = map_event(ev);
            std::visit(handler, internal_event);
            return true;
        }
        return false;
    }

    template <class...Fs>
    inline void handle_input(Fs&&... fs) {
        while (poll(std::forward<Fs>(fs)...));
    }
}

#endif //SDLPP_EVENTS_HH
