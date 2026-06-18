//
// Created by igor on 17/06/2026.
//

#include <simplex/simplex.hh>
#include <sdlpp/app/entry_point.hh>
#include <sdlpp/system/random.hh>
#include <algorithm>
#include <iostream>

using namespace simplex::literals;
using namespace sdlpp;

class example_circle_physics : public simplex::application {
    public:
        example_circle_physics() {
        }

    private:
        void on_ready() override {

            application::on_ready();
        }

        void on_update([[maybe_unused]] float delta_time) override {
        }

    private:
        random::engine rng;
};

SDLPP_MAIN(example_circle_physics)
