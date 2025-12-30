//
// High-level font API for SDL++ - implementation
//

#include <sdlpp/font/font.hh>
#include <sdlpp/font/sdl_raster_target.hh>

#include <onyx_font/font_factory.hh>
#include <onyx_font/bitmap_font.hh>
#include <onyx_font/vector_font.hh>
#include <onyx_font/ttf_font.hh>

#include <fstream>
#include <cmath>
#include <algorithm>
#include <variant>

namespace sdlpp::font {

// ============================================================================
// Implementation Details
// ============================================================================

struct font::impl {
    std::variant<
        std::monostate,
        onyx_font::bitmap_font,
        onyx_font::vector_font,
        onyx_font::ttf_font
    > loaded_font;

    std::unique_ptr<onyx_font::font_source> source;
    std::unique_ptr<onyx_font::text_rasterizer> rasterizer;
};

// ============================================================================
// File Loading Helpers
// ============================================================================

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

// ============================================================================
// Constructor / Destructor
// ============================================================================

font::~font() = default;

font::font(font&&) noexcept = default;
font& font::operator=(font&&) noexcept = default;

// ============================================================================
// Static Factory Methods
// ============================================================================

expected<font, std::string> font::load(const std::filesystem::path& path, std::size_t index) {
    auto data = read_file(path);
    if (data.empty()) {
        return make_unexpected("Failed to read file: " + path.string());
    }

    auto result = load(data, index);
    if (result) {
        result->m_data = std::move(data);
    }
    return result;
}

expected<font, std::string> font::load(std::span<const std::uint8_t> data, std::size_t index) {
    font f;
    f.m_impl = std::make_unique<impl>();

    // Analyze the container
    f.m_container_info = onyx_font::font_factory::analyze(data);

    if (f.m_container_info.fonts.empty()) {
        return make_unexpected("No fonts found in data");
    }

    if (index >= f.m_container_info.fonts.size()) {
        return make_unexpected("Font index out of range: " + std::to_string(index) +
                               " (max: " + std::to_string(f.m_container_info.fonts.size() - 1) + ")");
    }

    const auto& entry = f.m_container_info.fonts[index];

    try {
        switch (entry.type) {
            case onyx_font::font_type::BITMAP: {
                auto bitmap = onyx_font::font_factory::load_bitmap(data, index);
                f.m_impl->loaded_font = std::move(bitmap);
                auto* bmp_ptr = &std::get<onyx_font::bitmap_font>(f.m_impl->loaded_font);
                f.m_impl->source = std::make_unique<onyx_font::font_source>(
                    onyx_font::font_source::from_bitmap(*bmp_ptr));
                f.m_size = static_cast<float>(bmp_ptr->get_metrics().pixel_height);
                break;
            }

            case onyx_font::font_type::VECTOR: {
                auto vec = onyx_font::font_factory::load_vector(data, index);
                f.m_impl->loaded_font = std::move(vec);
                auto* vec_ptr = &std::get<onyx_font::vector_font>(f.m_impl->loaded_font);
                f.m_impl->source = std::make_unique<onyx_font::font_source>(
                    onyx_font::font_source::from_vector(*vec_ptr));
                break;
            }

            case onyx_font::font_type::OUTLINE: {
                auto ttf = onyx_font::font_factory::load_ttf(data, index);
                f.m_impl->loaded_font = std::move(ttf);
                auto* ttf_ptr = &std::get<onyx_font::ttf_font>(f.m_impl->loaded_font);
                f.m_impl->source = std::make_unique<onyx_font::font_source>(
                    onyx_font::font_source::from_ttf(*ttf_ptr));
                break;
            }

            default:
                return make_unexpected("Unknown font type");
        }

        f.m_impl->rasterizer = std::make_unique<onyx_font::text_rasterizer>(
            std::move(*f.m_impl->source));
        f.m_impl->rasterizer->set_size(f.m_size);
        f.m_valid = true;

    } catch (const std::exception& e) {
        return make_unexpected(std::string("Failed to load font: ") + e.what());
    }

    return f;
}

expected<font, std::string> font::load_raw(
    std::span<const std::uint8_t> data,
    const onyx_font::raw_font_options& options) {

    font f;
    f.m_impl = std::make_unique<impl>();

    try {
        auto bitmap = onyx_font::font_factory::load_raw(data, options);
        f.m_size = static_cast<float>(bitmap.get_metrics().pixel_height);

        f.m_impl->loaded_font = std::move(bitmap);
        auto* bmp_ptr = &std::get<onyx_font::bitmap_font>(f.m_impl->loaded_font);

        f.m_impl->source = std::make_unique<onyx_font::font_source>(
            onyx_font::font_source::from_bitmap(*bmp_ptr));
        f.m_impl->rasterizer = std::make_unique<onyx_font::text_rasterizer>(
            std::move(*f.m_impl->source));
        f.m_impl->rasterizer->set_size(f.m_size);

        f.m_valid = true;

        // Create minimal container info
        onyx_font::font_entry entry;
        entry.name = options.name;
        entry.type = onyx_font::font_type::BITMAP;
        entry.pixel_height = options.char_height;
        f.m_container_info.format = onyx_font::container_format::FNT;
        f.m_container_info.fonts.push_back(entry);

    } catch (const std::exception& e) {
        return make_unexpected(std::string("Failed to load raw font: ") + e.what());
    }

    return f;
}

// ============================================================================
// Font Information
// ============================================================================

font_type font::type() const {
    if (!m_impl) return font_type::UNKNOWN;

    if (std::holds_alternative<onyx_font::bitmap_font>(m_impl->loaded_font)) {
        return font_type::BITMAP;
    } else if (std::holds_alternative<onyx_font::vector_font>(m_impl->loaded_font)) {
        return font_type::VECTOR;
    } else if (std::holds_alternative<onyx_font::ttf_font>(m_impl->loaded_font)) {
        return font_type::OUTLINE;
    }
    return font_type::UNKNOWN;
}

bool font::is_scalable() const {
    return type() != font_type::BITMAP;
}

float font::native_size() const {
    if (!m_impl) return 0.0f;

    if (auto* bmp = std::get_if<onyx_font::bitmap_font>(&m_impl->loaded_font)) {
        return static_cast<float>(bmp->get_metrics().pixel_height);
    }
    return 0.0f;
}

// ============================================================================
// Size and Style
// ============================================================================

void font::set_size(float pixels) {
    m_size = pixels;
    if (m_impl && m_impl->rasterizer) {
        m_impl->rasterizer->set_size(pixels);
    }
}

void font::set_style(text_style style) {
    if (m_impl && m_impl->rasterizer) {
        m_impl->rasterizer->set_style(style);
    }
}

void font::set_style(const render_style& style) {
    if (m_impl && m_impl->rasterizer) {
        m_impl->rasterizer->set_style(style);
    }
}

text_style font::style() const {
    if (m_impl && m_impl->rasterizer) {
        return m_impl->rasterizer->style();
    }
    return text_style::normal;
}

void font::reset_style() {
    if (m_impl && m_impl->rasterizer) {
        m_impl->rasterizer->reset_style();
    }
}

// ============================================================================
// Measurement
// ============================================================================

text_metrics font::measure(std::string_view text) const {
    text_metrics result{};

    if (!m_impl || !m_impl->rasterizer) {
        return result;
    }

    auto extents = m_impl->rasterizer->measure_text(text);
    result.width = extents.width;
    result.height = extents.height;
    result.ascent = extents.ascent;
    result.descent = extents.descent;
    result.line_height = m_impl->rasterizer->line_height();

    return result;
}

text_metrics font::measure_wrapped(std::string_view text, float max_width) const {
    text_metrics result{};

    if (!m_impl || !m_impl->rasterizer) {
        return result;
    }

    auto extents = m_impl->rasterizer->measure_wrapped(text, max_width);
    result.width = extents.width;
    result.height = extents.height;
    result.ascent = extents.ascent;
    result.descent = extents.descent;
    result.line_height = m_impl->rasterizer->line_height();

    return result;
}

text_metrics font::get_metrics() const {
    text_metrics result{};

    if (!m_impl || !m_impl->rasterizer) {
        return result;
    }

    auto metrics = m_impl->rasterizer->get_metrics();
    result.ascent = metrics.ascent;
    result.descent = metrics.descent;
    result.line_height = metrics.line_height;
    result.height = metrics.ascent + metrics.descent;

    return result;
}

float font::line_height() const {
    if (m_impl && m_impl->rasterizer) {
        return m_impl->rasterizer->line_height();
    }
    return m_size;
}

// ============================================================================
// Rendering
// ============================================================================

expected<surface, std::string> font::render_text(
    std::string_view text,
    const color& fg,
    const color& bg) const {

    if (!m_impl || !m_impl->rasterizer) {
        return make_unexpected("Font not initialized");
    }

    if (text.empty()) {
        return make_unexpected("Empty text");
    }

    // Measure the text
    auto extents = m_impl->rasterizer->measure_text(text);
    int width = static_cast<int>(std::ceil(extents.width));
    int height = static_cast<int>(std::ceil(extents.height));

    if (width <= 0 || height <= 0) {
        return make_unexpected("Invalid text dimensions");
    }

    // Create surface - use ABGR8888 which stores bytes as R,G,B,A on little-endian
    // This matches surface_raster_target::put_pixel's byte order expectations
    auto surf_result = surface::create_rgb(width, height, pixel_format_enum::ABGR8888);
    if (!surf_result) {
        return make_unexpected("Failed to create surface: " + surf_result.error());
    }

    auto& surf = *surf_result;

    // Fill background
    surf.fill_rect(rect<int>(0, 0, width, height), bg);

    // Render text
    surface_raster_target target(surf, fg);
    int baseline = static_cast<int>(std::ceil(extents.ascent));
    m_impl->rasterizer->rasterize_text(text, target, 0, baseline);

    return surf_result;
}

expected<texture, std::string> font::render_texture(
    renderer& renderer,
    std::string_view text,
    const color& fg) const {

    auto surf_result = render_text(text, fg, {0, 0, 0, 0});
    if (!surf_result) {
        return make_unexpected(surf_result.error());
    }

    return texture::create(renderer, *surf_result);
}

float font::render_to(
    surface& target,
    std::string_view text,
    int x, int y,
    const color& fg) const {

    if (!m_impl || !m_impl->rasterizer) {
        return 0.0f;
    }

    surface_raster_target raster_target(target, fg);

    // Calculate baseline from top position
    auto metrics = m_impl->rasterizer->get_metrics();
    int baseline = y + static_cast<int>(std::ceil(metrics.ascent));

    return m_impl->rasterizer->rasterize_text(text, raster_target, x, baseline);
}

// ============================================================================
// Access to Underlying Objects
// ============================================================================

onyx_font::text_rasterizer* font::rasterizer() {
    return m_impl ? m_impl->rasterizer.get() : nullptr;
}

const onyx_font::text_rasterizer* font::rasterizer() const {
    return m_impl ? m_impl->rasterizer.get() : nullptr;
}

// ============================================================================
// Utility Functions
// ============================================================================

std::vector<std::string> supported_formats() {
    return {
        "TTF (TrueType Font)",
        "OTF (OpenType Font)",
        "TTC (TrueType Collection)",
        "FON (Windows Font)",
        "FNT (Windows Font Resource)",
        "CHR (Borland BGI Font)",
        "GEM (GEM Font)",
        "Raw BIOS Font"
    };
}

expected<onyx_font::container_info, std::string> analyze(const std::filesystem::path& path) {
    auto data = read_file(path);
    if (data.empty()) {
        return make_unexpected("Failed to read file: " + path.string());
    }

    return onyx_font::font_factory::analyze(data);
}

onyx_font::container_info analyze(std::span<const std::uint8_t> data) {
    return onyx_font::font_factory::analyze(data);
}

} // namespace sdlpp::font
