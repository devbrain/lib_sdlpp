//
// High-level image loading API for SDL++
//

#include <sdlpp/image/image.hh>
#include <fstream>
#include <limits>
#include <cmath>

namespace sdlpp::image {

namespace {

std::vector<std::uint8_t> read_file(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return {};
    }

    auto size = file.tellg();
    file.seekg(0);

    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        return {};
    }

    return data;
}

} // anonymous namespace

expected<surface, std::string> load(const std::filesystem::path& path) {
    auto data = read_file(path);
    if (data.empty()) {
        return make_unexpectedf("Failed to read file:", path.string());
    }

    return load(data);
}

expected<surface, std::string> load(std::span<const std::uint8_t> data) {
    sdlpp::surface result;
    sdl_surface_adapter adapter(result);

    auto decode_result = onyx_image::decode(data, adapter);
    if (!decode_result) {
        return make_unexpectedf("Failed to decode image:", std::string(onyx_image::to_string(decode_result.error)));
    }

    return result;
}

expected<texture, std::string> load_texture(
    renderer& renderer,
    const std::filesystem::path& path) {

    auto surface_result = load(path);
    if (!surface_result) {
        return make_unexpectedf(surface_result.error());
    }

    return texture::create(renderer, *surface_result);
}

expected<texture, std::string> load_texture(
    renderer& renderer,
    std::span<const std::uint8_t> data) {

    auto surface_result = load(data);
    if (!surface_result) {
        return make_unexpectedf(surface_result.error());
    }

    return texture::create(renderer, *surface_result);
}

expected<surface, std::string> decode(
    std::span<const std::uint8_t> data,
    const std::string& codec_name) {

    sdlpp::surface result;
    sdl_surface_adapter adapter(result);

    auto decode_result = onyx_image::decode(data, adapter, codec_name);
    if (!decode_result) {
        return make_unexpectedf("Failed to decode image with codec '", codec_name, "':", std::string(onyx_image::to_string(decode_result.error)));
    }

    return result;
}

std::vector<std::string> supported_formats() {
    std::vector<std::string> formats;
    auto& registry = onyx_image::codec_registry::instance();
    for (std::size_t i = 0; i < registry.decoder_count(); ++i) {
        if (auto* dec = registry.decoder_at(i)) {
            formats.emplace_back(dec->name());
        }
    }
    return formats;
}

bool is_format_supported(const std::string& codec_name) {
    auto& registry = onyx_image::codec_registry::instance();
    return registry.find_decoder(codec_name) != nullptr;
}

// ----------------------------------------------------------------------------
// Multi-image container support
// ----------------------------------------------------------------------------

expected<surface, std::string> image_atlas::extract(std::size_t index) const {
    if (index >= regions.size()) {
        return make_unexpectedf("Image index out of range");
    }

    const auto& region = regions[index];

    auto result = surface::create_rgb(
        region.bounds.w, region.bounds.h, atlas.format());
    if (!result) {
        return make_unexpectedf("Failed to create surface:", result.error());
    }

    // Blit the region from atlas to new surface
    auto blit_result = atlas.blit_to(*result, std::optional{region.bounds}, point<int>{0, 0});
    if (!blit_result) {
        return make_unexpectedf("Failed to blit region:", blit_result.error());
    }

    return result;
}

std::optional<std::size_t> image_atlas::find_best_size(int target_size) const {
    if (regions.empty()) return std::nullopt;

    std::size_t best_idx = 0;
    int best_diff = std::numeric_limits<int>::max();

    for (std::size_t i = 0; i < regions.size(); ++i) {
        int size = std::max(regions[i].bounds.w, regions[i].bounds.h);
        int diff = std::abs(size - target_size);
        if (diff < best_diff) {
            best_diff = diff;
            best_idx = i;
        }
    }

    return best_idx;
}

expected<image_atlas, std::string> load_atlas(const std::filesystem::path& path) {
    auto data = read_file(path);
    if (data.empty()) {
        return make_unexpectedf("Failed to read file:", path.string());
    }

    return load_atlas(data);
}

expected<image_atlas, std::string> load_atlas(std::span<const std::uint8_t> data) {
    sdlpp::surface result_surface;
    sdl_surface_adapter adapter(result_surface);

    auto decode_result = onyx_image::decode(data, adapter);
    if (!decode_result) {
        return make_unexpectedf("Failed to decode:", std::string(onyx_image::to_string(decode_result.error)));
    }

    image_atlas result;
    result.atlas = std::move(result_surface);

    // Convert subrects to image_regions
    for (const auto& sr : adapter.subrects()) {
        image_region region;
        region.bounds = rect<int>{sr.rect.x, sr.rect.y, sr.rect.w, sr.rect.h};
        region.type = static_cast<image_region::kind>(sr.kind);
        region.index = sr.user_tag;
        result.regions.push_back(region);
    }

    // If no subrects were reported, treat the whole image as one region
    if (result.regions.empty() && result.atlas) {
        image_region region;
        region.bounds = rect<int>{0, 0,
                                  static_cast<int>(result.atlas.width()),
                                  static_cast<int>(result.atlas.height())};
        region.type = image_region::kind::sprite;
        region.index = 0;
        result.regions.push_back(region);
    }

    return result;
}

} // namespace sdlpp::image
