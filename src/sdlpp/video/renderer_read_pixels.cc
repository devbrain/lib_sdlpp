/**
 * @file renderer_read_pixels.cc
 * @brief renderer::read_pixels(), defined here because it needs the complete surface type
 */

#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/surface.hh>

namespace sdlpp {
    expected <surface, std::string> renderer::read_pixels() const {
        if (!ptr) {
            return make_unexpectedf("Invalid renderer");
        }
        SDL_Surface* pixels = SDL_RenderReadPixels(ptr.get(), nullptr);
        if (!pixels) {
            return make_unexpectedf(get_error());
        }
        return surface(pixels);
    }
}
