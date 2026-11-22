#include <doctest/doctest.h>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/texture.hh>
#include <sdlpp/video/surface.hh>

#include <optional>
#include <utility>

TEST_CASE("renderer state wrappers reject an empty renderer") {
    sdlpp::renderer renderer;
    CHECK_FALSE(renderer.set_draw_color_float({0.1f, 0.2f, 0.3f, 1.0f}));
    CHECK_FALSE(renderer.get_draw_color_float());
    CHECK_FALSE(renderer.set_logical_presentation(20, 10, sdlpp::logical_presentation::integer_scale));
    CHECK_FALSE(renderer.get_logical_presentation());
    CHECK_FALSE(renderer.coordinates_from_window(sdlpp::point_f{1.0f, 1.0f}));
    CHECK_FALSE(renderer.coordinates_to_window(sdlpp::point_f{1.0f, 1.0f}));
    CHECK_FALSE(renderer.get_max_texture_size());
    CHECK_FALSE(renderer.read_pixels());
    CHECK_FALSE(renderer.get_target());
    CHECK_FALSE(renderer.set_draw_color({1, 2, 3, 4}));
    sdlpp::renderer::target_guard guard(renderer);
    CHECK_FALSE(guard.restore());
}

TEST_CASE("renderer state wrappers preserve precision and target state") {
    auto surface = sdlpp::surface::create_rgb(80, 40, sdlpp::pixel_format_enum::RGBA8888);
    REQUIRE(surface);
    auto renderer = sdlpp::renderer::create_software(surface->get());
    REQUIRE(renderer);

    SUBCASE("floating-point draw color is not quantized") {
        const sdlpp::fcolor color{0.1234f, 0.4567f, 0.7891f, 0.6543f};
        REQUIRE(renderer->set_draw_color_float(color));
        const auto actual = renderer->get_draw_color_float();
        REQUIRE(actual);
        CHECK(actual->r == doctest::Approx(color.r));
        CHECK(actual->g == doctest::Approx(color.g));
        CHECK(actual->b == doctest::Approx(color.b));
        CHECK(actual->a == doctest::Approx(color.a));
    }

    SUBCASE("logical presentation belongs to each target") {
        REQUIRE(renderer->set_logical_presentation(20, 10, sdlpp::logical_presentation::integer_scale));
        auto canvas = sdlpp::texture::create(*renderer, sdlpp::pixel_format_enum::RGBA8888,
                                           sdlpp::texture_access::target, 16, 16);
        REQUIRE(canvas);
        {
            sdlpp::renderer::target_guard guard(*renderer, *canvas);
            REQUIRE(renderer->set_logical_presentation(8, 8, sdlpp::logical_presentation::integer_scale));
            const auto inner = renderer->get_logical_presentation();
            REQUIRE(inner);
            CHECK(inner->width == 8);
            CHECK(inner->height == 8);
            CHECK(inner->mode == sdlpp::logical_presentation::integer_scale);
        }
        const auto outer = renderer->get_logical_presentation();
        REQUIRE(outer);
        CHECK(outer->width == 20);
        CHECK(outer->height == 10);
        CHECK(outer->mode == sdlpp::logical_presentation::integer_scale);
    }

    SUBCASE("a target with no logical presentation reports it disabled") {
        const auto info = renderer->get_logical_presentation();
        REQUIRE(info);
        CHECK(info->mode == sdlpp::logical_presentation::disabled);
    }

    SUBCASE("coordinates map through the logical presentation and back") {
        // 80x40 output, 20x10 logical, stretched: one logical pixel is 4x4 output pixels.
        REQUIRE(renderer->set_logical_presentation(20, 10, sdlpp::logical_presentation::stretch));
        const auto render = renderer->coordinates_from_window(sdlpp::point_f{40.0f, 20.0f});
        REQUIRE(render);
        CHECK(render->x == doctest::Approx(10.0f));
        CHECK(render->y == doctest::Approx(5.0f));
        const auto window = renderer->coordinates_to_window(*render);
        REQUIRE(window);
        CHECK(window->x == doctest::Approx(40.0f));
        CHECK(window->y == doctest::Approx(20.0f));
    }

    SUBCASE("the target size includes the letterbox bars; the current output size does not") {
        // 80x40 output, 20x40 logical, letterboxed: bars left and right, the picture 20x40.
        REQUIRE(renderer->set_logical_presentation(20, 40, sdlpp::logical_presentation::letterbox));
        const auto current = renderer->get_current_output_size();
        REQUIRE(current);
        CHECK(current->width == 20);
        CHECK(current->height == 40);
        const auto whole = renderer->get_target_size();
        REQUIRE(whole);
        CHECK(whole->width == 80);
        CHECK(whole->height == 40);

        auto canvas = sdlpp::texture::create(*renderer, sdlpp::pixel_format_enum::RGBA8888,
                                           sdlpp::texture_access::target, 16, 8);
        REQUIRE(canvas);
        {
            sdlpp::renderer::target_guard guard(*renderer, *canvas);
            REQUIRE(renderer->set_logical_presentation(8, 8, sdlpp::logical_presentation::letterbox));
            const auto inner = renderer->get_target_size();
            REQUIRE(inner);
            CHECK(inner->width == 16);
            CHECK(inner->height == 8);
            const auto inner_current = renderer->get_current_output_size();
            REQUIRE(inner_current);
            CHECK(inner_current->width == 8);
        }
        REQUIRE(renderer->set_logical_presentation(0, 0, sdlpp::logical_presentation::disabled));
        const auto plain = renderer->get_target_size();
        REQUIRE(plain);
        CHECK(plain->width == 80);
    }

    SUBCASE("max texture size and properties are reported") {
        CHECK(renderer->get_properties());
        const auto size = renderer->get_max_texture_size();
        REQUIRE(size);
        CHECK(*size >= 0);
    }

    SUBCASE("read_pixels reads the current target") {
        REQUIRE(renderer->set_draw_color({255, 0, 0, 255}));
        REQUIRE(renderer->clear());
        auto pixels = renderer->read_pixels();
        REQUIRE(pixels);
        CHECK(pixels->width() == 80);
        CHECK(pixels->height() == 40);
        const auto c = pixels->get_pixel(3, 3);
        REQUIRE(c);
        CHECK(c->r == 255);
        CHECK(c->g == 0);
        CHECK(c->b == 0);
    }

    SUBCASE("get_target borrows the target instead of owning it") {
        auto canvas = sdlpp::texture::create(*renderer, sdlpp::pixel_format_enum::RGBA8888,
                                           sdlpp::texture_access::target, 16, 16);
        REQUIRE(canvas);
        REQUIRE(renderer->set_target(*canvas));
        for (int i = 0; i < 2; ++i) {
            const auto current = renderer->get_target();
            REQUIRE(current);
            CHECK(*current == canvas->get());
        }
        // Still alive: the queries above did not destroy it.
        CHECK(canvas->get_size());
        REQUIRE(renderer->clear());
        REQUIRE(renderer->set_target(sdlpp::texture{}));
        const auto window = renderer->get_target();
        REQUIRE(window);
        CHECK(*window == nullptr);
    }

    SUBCASE("a moved target_guard restores once, from its new owner") {
        auto first = sdlpp::texture::create(*renderer, sdlpp::pixel_format_enum::RGBA8888,
                                          sdlpp::texture_access::target, 16, 16);
        auto second = sdlpp::texture::create(*renderer, sdlpp::pixel_format_enum::RGBA8888,
                                           sdlpp::texture_access::target, 16, 16);
        REQUIRE(first);
        REQUIRE(second);
        REQUIRE(renderer->set_target(*first));
        std::optional <sdlpp::renderer::target_guard> held;
        held.emplace(*renderer, *second);
        CHECK(SDL_GetRenderTarget(renderer->get()) == second->get());
        sdlpp::renderer::target_guard moved = std::move(*held);
        held.reset();                                   // moved-from: restores nothing
        CHECK(SDL_GetRenderTarget(renderer->get()) == second->get());
        REQUIRE(moved.restore());
        CHECK(SDL_GetRenderTarget(renderer->get()) == first->get());

        // Move assignment restores the assigned-to guard's target before taking over.
        REQUIRE(renderer->set_target(sdlpp::texture{}));
        sdlpp::renderer::target_guard to_window(*renderer, *first);   // saved: window
        {
            sdlpp::renderer::target_guard to_first(*renderer, *second); // saved: first
            to_window = std::move(to_first);                          // restores window
            CHECK(SDL_GetRenderTarget(renderer->get()) == nullptr);
        }
        REQUIRE(to_window.restore());                                 // now restores first
        CHECK(SDL_GetRenderTarget(renderer->get()) == first->get());
        REQUIRE(renderer->set_target(sdlpp::texture{}));
    }

    SUBCASE("saved targets are borrowed and early restoration releases the guard") {
        auto first = sdlpp::texture::create(*renderer, sdlpp::pixel_format_enum::RGBA8888,
                                          sdlpp::texture_access::target, 16, 16);
        auto second = sdlpp::texture::create(*renderer, sdlpp::pixel_format_enum::RGBA8888,
                                           sdlpp::texture_access::target, 16, 16);
        REQUIRE(first);
        REQUIRE(second);
        REQUIRE(renderer->set_target(*first));
        {
            sdlpp::renderer::target_guard saved(*renderer);
            CHECK(SDL_GetRenderTarget(renderer->get()) == first->get());
            {
                sdlpp::renderer::target_guard nested(*renderer, *second);
                CHECK(SDL_GetRenderTarget(renderer->get()) == second->get());
            }
            CHECK(SDL_GetRenderTarget(renderer->get()) == first->get());
            REQUIRE(renderer->set_target(*second));
            REQUIRE(saved.restore());
            CHECK(SDL_GetRenderTarget(renderer->get()) == first->get());
            REQUIRE(saved.restore());
            REQUIRE(renderer->set_target(sdlpp::texture{}));
        }
        CHECK(SDL_GetRenderTarget(renderer->get()) == nullptr);
        // Neither guard may destroy the borrowed target.
        REQUIRE(renderer->set_target(*first));
        REQUIRE(renderer->set_target(*second));
        REQUIRE(renderer->clear());
        REQUIRE(renderer->set_target(sdlpp::texture{}));
    }
}
