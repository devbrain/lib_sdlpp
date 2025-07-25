#include <sdlpp/video/camera.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/texture.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/core/core.hh>
#include <../include/sdlpp/core/timer.hh>
#include <../include/sdlpp/core/log.hh>
#include <sdlpp/utility/geometry.hh>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <optional>

using namespace sdlpp;
using namespace std::chrono_literals;

// Helper to display camera information
void display_camera_info(const camera& cam) {
    std::cout << "\n=== Camera Information ===\n";
    std::cout << "Name: " << cam.get_name() << "\n";
    std::cout << "ID: " << cam.get_id() << "\n";
    
    auto pos = cam.get_position();
    std::cout << "Position: ";
    switch (pos) {
        case camera_position::front_facing:
            std::cout << "Front-facing\n";
            break;
        case camera_position::back_facing:
            std::cout << "Back-facing\n";
            break;
        default:
            std::cout << "Unknown\n";
            break;
    }
    
    auto format = cam.get_format();
    if (format) {
        std::cout << "Current format: " << format->width << "x" << format->height
                  << " @ " << format->get_framerate() << " FPS"
                  << " (format: " << static_cast<int>(format->format) << ")\n";
    }
    
    // Permission handling is now at system level in SDL3
    std::cout << "Permission status: Handled by system\n";
    
    std::cout << "\nSupported formats:\n";
    auto formats = cam.get_supported_formats();
    for (size_t i = 0; i < formats.size(); ++i) {
        const auto& fmt = formats[i];
        std::cout << "  [" << i << "] " << fmt.width << "x" << fmt.height
                  << " @ " << fmt.get_framerate() << " FPS"
                  << " (format: " << static_cast<int>(fmt.format) << ")\n";
    }
}

// Camera preview window
class camera_preview {
private:
    window& win;
    renderer& ren;
    camera& cam;
    std::optional<texture> preview_texture;
    
    // Statistics
    uint64_t frame_count = 0;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_fps_update;
    double current_fps = 0.0;
    
public:
    camera_preview(window& w, renderer& r, camera& c) 
        : win(w), ren(r), cam(c), 
          start_time(std::chrono::steady_clock::now()),
          last_fps_update(start_time) {}
    
    bool update() {
        // Acquire frame from camera
        camera_frame frame(cam);
        if (!frame) {
            return false;  // No frame available
        }
        
        // Update statistics
        frame_count++;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - last_fps_update).count();
        if (elapsed >= 1.0) {
            current_fps = static_cast<double>(frame_count) / elapsed;
            frame_count = 0;
            last_fps_update = now;
        }
        
        // Create or update texture
        auto tex_size = preview_texture ? preview_texture->get_size() : expected<size_i, std::string>{};
        if (!preview_texture || !tex_size ||
            tex_size->width != frame->w ||
            tex_size->height != frame->h) {
            
            // Need to create new texture
            auto tex = texture::create(ren, 
                static_cast<pixel_format_enum>(frame->format),
                texture_access::streaming,
                frame->w, frame->h);
            
            if (tex) {
                preview_texture = std::move(*tex);
            } else {
                std::cerr << "Failed to create texture: " << tex.error() << "\n";
                return false;
            }
        }
        
        // Update texture with frame data
        if (preview_texture) {
            auto result = preview_texture->update(
                std::optional<rect_i>{},  // Update entire texture
                frame->pixels,
                frame->pitch);
            
            if (!result) {
                std::cerr << "Failed to update texture: " << result.error() << "\n";
            }
        }
        
        return true;
    }
    
    void render() {
        // Clear renderer
        auto color_result = ren.set_draw_color({0, 0, 0, 255});
        if (!color_result) return;
        auto clear_result = ren.clear();
        if (!clear_result) return;
        
        if (preview_texture) {
            // Calculate centered position
            auto win_size_result = win.get_size();
            if (!win_size_result) return;
            auto win_size = *win_size_result;
            
            auto tex_size_result = preview_texture->get_size();
            if (!tex_size_result) return;
            auto tex_size = *tex_size_result;
            
            // Scale to fit window while maintaining aspect ratio
            float scale_x = static_cast<float>(win_size.width) / static_cast<float>(tex_size.width);
            float scale_y = static_cast<float>(win_size.height) / static_cast<float>(tex_size.height);
            float scale = std::min(scale_x, scale_y);
            
            int scaled_w = static_cast<int>(static_cast<float>(tex_size.width) * scale);
            int scaled_h = static_cast<int>(static_cast<float>(tex_size.height) * scale);
            
            rect_f dst_rect(
                static_cast<float>(win_size.width - scaled_w) / 2.0f,
                static_cast<float>(win_size.height - scaled_h) / 2.0f,
                static_cast<float>(scaled_w),
                static_cast<float>(scaled_h)
            );
            
            // Render texture
            auto copy_result = ren.copy(*preview_texture, std::optional<rect_f>{}, std::optional<rect_f>{dst_rect});
            if (!copy_result) return;
        }
        
        // Show FPS in window title
        std::stringstream title;
        title << "Camera Preview - " << std::fixed << std::setprecision(1) << current_fps << " FPS";
        auto title_result = win.set_title(title.str());
        if (!title_result) return;
        
        // Present
        auto present_result = ren.present();
        if (!present_result) return;
    }
};

int main() {
    std::cout << "=== SDL++ Camera Example ===\n\n";
    
    // Initialize SDL with camera and video support
    auto init_result = init(init_flags::video | init_flags::camera);
    if (!init_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to initialize SDL");
        return 1;
    }
    
    // List available cameras
    auto cameras = get_cameras();
    if (cameras.empty()) {
        std::cout << "No cameras found!\n";
        std::cout << "Make sure your camera is connected and permissions are granted.\n";
        return 0;
    }
    
    std::cout << "Available cameras:\n";
    for (size_t i = 0; i < cameras.size(); ++i) {
        auto name = get_camera_name(cameras[i]);
        auto position = get_camera_position(cameras[i]);
        
        std::cout << "  [" << i << "] " << name;
        if (position == camera_position::front_facing) {
            std::cout << " (front)";
        } else if (position == camera_position::back_facing) {
            std::cout << " (back)";
        }
        std::cout << "\n";
    }
    
    // Select camera
    size_t selected = 0;
    if (cameras.size() > 1) {
        std::cout << "\nSelect camera (0-" << cameras.size() - 1 << "): ";
        std::cin >> selected;
        std::cin.ignore();
        
        if (selected >= cameras.size()) {
            selected = 0;
        }
    }
    
    // Open camera
    auto cam_result = camera::open(cameras[selected]);
    if (!cam_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to open camera: " + cam_result.error());
        return 1;
    }
    
    auto& cam = *cam_result;
    display_camera_info(cam);
    
    // Permission is now handled at system level in SDL3
    
    // Ask about format selection
    std::cout << "\nUse custom format? (y/n): ";
    char use_custom;
    std::cin >> use_custom;
    std::cin.ignore();
    
    if (use_custom == 'y' || use_custom == 'Y') {
        auto formats = cam.get_supported_formats();
        if (!formats.empty()) {
            std::cout << "Select format (0-" << formats.size() - 1 << "): ";
            size_t fmt_idx;
            std::cin >> fmt_idx;
            std::cin.ignore();
            
            if (fmt_idx < formats.size()) {
                // Close and reopen with specific format
                cam = camera();  // Close current
                cam_result = camera::open(cameras[selected], &formats[fmt_idx]);
                if (!cam_result) {
                    logger::error(log_category::application, std::source_location::current(), 
                                 "Failed to reopen camera with format");
                    return 1;
                }
                cam = std::move(*cam_result);
                std::cout << "Camera reopened with selected format.\n";
            }
        }
    }
    
    // Create window for preview
    auto format = cam.get_format();
    int window_width = 800;
    int window_height = 600;
    
    if (format) {
        // Use camera resolution for window, but cap at reasonable size
        window_width = std::min(static_cast<int>(format->width), 1280);
        window_height = std::min(static_cast<int>(format->height), 720);
    }
    
    auto window_result = window::create("Camera Preview", window_width, window_height);
    if (!window_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to create window");
        return 1;
    }
    
    auto& win = *window_result;
    
    // Create renderer
    auto renderer_result = renderer::create(win);
    if (!renderer_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to create renderer");
        return 1;
    }
    
    auto& ren = *renderer_result;
    
    // Create preview handler
    camera_preview preview(win, ren, cam);
    
    std::cout << "\nStarting camera preview...\n";
    std::cout << "Press ESC or close window to exit.\n";
    std::cout << "Press SPACE to save a snapshot (not implemented in this example).\n";
    
    // Main loop
    bool running = true;
    frame_limiter limiter(60.0);  // Cap at 60 FPS for display
    
    while (running) {
        // Handle events
        while (auto e = event_queue::poll()) {
            if (e->type() == event_type::quit) {
                running = false;
            } else if (e->type() == event_type::key_down) {
                auto* key_event = e->as<keyboard_event>();
                if (key_event && key_event->scan == scancode::escape) {
                    running = false;
                } else if (key_event && key_event->scan == scancode::space) {
                    std::cout << "Snapshot feature not implemented in this example.\n";
                }
            }
        }
        
        // Update camera frame
        preview.update();
        
        // Render
        preview.render();
        
        // Frame rate limiting
        limiter.wait_for_next_frame();
    }
    
    std::cout << "\nCamera preview ended.\n";
    return 0;
}