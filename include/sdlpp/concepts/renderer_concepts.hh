//
// Created by igor on 7/27/25.
//

#pragma once

/**
 * @file renderer_concepts.hh
 * @brief Concepts for renderer interfaces in SDL++
 * 
 * This header defines concepts that generalize the common interface between
 * hardware renderer and software surface_renderer, allowing generic algorithms
 * to work with either rendering backend.
 */

#include <sdlpp/detail/expected.hh>
#include <sdlpp/utility/geometry_concepts.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/video/blend_mode.hh>

// Euler includes (with warning suppression)
#include <sdlpp/core/euler.hh>

#include <concepts>
#include <optional>
#include <string>

namespace sdlpp {

/**
 * @brief Basic renderer concept with core functionality
 * 
 * This concept defines the minimum interface that all renderers must support
 */
template<typename T>
concept basic_renderer = requires(T& r, const T& cr) {
    // Basic validity check
    { cr.is_valid() } -> std::convertible_to<bool>;
    
    // Clear operation
    { r.clear() } -> std::same_as<expected<void, std::string>>;
} && requires(T& r, const T& cr, const color& c) {
    // Color management
    { r.set_draw_color(c) } -> std::same_as<expected<void, std::string>>;
    { cr.get_draw_color() } -> std::same_as<expected<color, std::string>>;
} && requires(T& r, const T& cr, blend_mode mode) {
    // Blend mode management
    { r.set_draw_blend_mode(mode) } -> std::same_as<expected<void, std::string>>;
    { cr.get_draw_blend_mode() } -> std::same_as<expected<blend_mode, std::string>>;
};

/**
 * @brief Concept for renderers with primitive drawing capabilities
 * 
 * This includes point, line, and shape drawing
 */
template<typename T>
concept primitive_renderer = basic_renderer<T> && requires(T& r) {
    // Template methods must work with geometry concepts
    // We check this by verifying methods exist that can be called
    // with some concrete instantiation
    
    // Point drawing - check with int coordinates
    requires requires(int x, int y) {
        { r.draw_point(x, y) } -> std::same_as<expected<void, std::string>>;
    };
    
    // Line drawing - check with int coordinates
    requires requires(int x1, int y1, int x2, int y2) {
        { r.draw_line(x1, y1, x2, y2) } -> std::same_as<expected<void, std::string>>;
    };
    
    // Rectangle operations - check with int coordinates
    requires requires(int x, int y, int w, int h) {
        { r.draw_rect(x, y, w, h) } -> std::same_as<expected<void, std::string>>;
        { r.fill_rect(x, y, w, h) } -> std::same_as<expected<void, std::string>>;
    };
};

/**
 * @brief Concept for renderers with DDA (Digital Differential Analyzer) support
 */
template<typename T>
concept dda_renderer = primitive_renderer<T> && requires(T& r) {
    // Antialiased lines - check with float coordinates
    requires requires(float x1, float y1, float x2, float y2) {
        { r.draw_line_aa(x1, y1, x2, y2) } -> std::same_as<expected<void, std::string>>;
    };
    
    // Thick lines
    requires requires(float x1, float y1, float x2, float y2, float thickness) {
        { r.draw_line_thick(x1, y1, x2, y2, thickness) } -> std::same_as<expected<void, std::string>>;
    };
    
    // Circles
    requires requires(int x, int y, int radius) {
        { r.draw_circle(x, y, radius) } -> std::same_as<expected<void, std::string>>;
        { r.fill_circle(x, y, radius) } -> std::same_as<expected<void, std::string>>;
    };
    
    // Ellipses
    requires requires(int x, int y, int rx, int ry) {
        { r.draw_ellipse(x, y, rx, ry) } -> std::same_as<expected<void, std::string>>;
        { r.fill_ellipse(x, y, rx, ry) } -> std::same_as<expected<void, std::string>>;
    };
    
    // Ellipse arcs
    requires requires(int x, int y, int rx, int ry, float start_angle, float end_angle) {
        { r.draw_ellipse_arc(x, y, rx, ry, start_angle, end_angle) } -> std::same_as<expected<void, std::string>>;
    };
};

/**
 * @brief Concept for renderers with Bezier curve support
 */
template<typename T>
concept bezier_renderer = dda_renderer<T> && requires(T& r) {
    // Quadratic Bezier
    requires requires(float x0, float y0, float x1, float y1, float x2, float y2) {
        { r.draw_bezier_quad(x0, y0, x1, y1, x2, y2) } -> std::same_as<expected<void, std::string>>;
    };
    
    // Cubic Bezier
    requires requires(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
        { r.draw_bezier_cubic(x0, y0, x1, y1, x2, y2, x3, y3) } -> std::same_as<expected<void, std::string>>;
    };
};

/**
 * @brief Concept for renderers with clipping support
 */
template<typename T>
concept clipping_renderer = basic_renderer<T> && requires(T& r, const T& cr) {
    // Check that is_clip_enabled exists
    { cr.is_clip_enabled() } -> std::convertible_to<bool>;
};

/**
 * @brief Concept for renderers with euler angle support
 */
template<typename T>
concept euler_angle_renderer = dda_renderer<T> && requires(T& r) {
    // Check for euler angle overloads
    requires requires(int x, int y, int rx, int ry, euler::radian<float> start, euler::radian<float> end) {
        { r.draw_ellipse_arc(x, y, rx, ry, start, end) } -> std::same_as<expected<void, std::string>>;
    };
};

/**
 * @brief Full-featured renderer concept
 * 
 * Combines all renderer capabilities
 */
template<typename T>
concept renderer_like = bezier_renderer<T> && clipping_renderer<T> && euler_angle_renderer<T>;

/**
 * @brief Simplified renderer concept for basic use cases
 */
template<typename T>
concept simple_renderer = primitive_renderer<T>;

} // namespace sdlpp