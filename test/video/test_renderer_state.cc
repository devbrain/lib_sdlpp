#include <doctest/doctest.h>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/texture.hh>

TEST_CASE("renderer state wrappers reject an empty renderer") {
    sdlpp::renderer renderer;
    CHECK_FALSE(renderer.set_draw_color_float({0.1f, 0.2f, 0.3f, 1.0f}));
    CHECK_FALSE(renderer.get_draw_color_float());
    CHECK_FALSE(renderer.set_logical_presentation(20, 10, sdlpp::logical_presentation::integer_scale));
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
            int width = 0, height = 0;
            SDL_RendererLogicalPresentation mode{};
            REQUIRE(SDL_GetRenderLogicalPresentation(renderer->get(), &width, &height, &mode));
            CHECK(width == 8);
            CHECK(height == 8);
            CHECK(mode == SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
        }
        int width = 0, height = 0;
        SDL_RendererLogicalPresentation mode{};
        REQUIRE(SDL_GetRenderLogicalPresentation(renderer->get(), &width, &height, &mode));
        CHECK(width == 20);
        CHECK(height == 10);
        CHECK(mode == SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
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
