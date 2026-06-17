//
// Created by igor on 2026-06-17.
//

#pragma once

namespace sdlpp {

    /**
     * @brief Type of line styling
     */
    enum class line_style_type {
        solid,
        dashed,
        dotted
    };

    /**
     * @brief Style definition for stateful line drawing
     */
    struct line_style {
        line_style_type type = line_style_type::solid;
        float dash = 8.0f;
        float gap = 8.0f;
        float spacing = 4.0f;

        /**
         * @brief Create a solid line style
         */
        static line_style solid() {
            line_style style;
            style.type = line_style_type::solid;
            return style;
        }

        /**
         * @brief Create a dashed line style
         * @param dash Length of solid dash in pixels
         * @param gap Length of gap in pixels
         */
        static line_style dashed(float dash = 8.0f, float gap = 8.0f) {
            line_style style;
            style.type = line_style_type::dashed;
            style.dash = dash;
            style.gap = gap;
            return style;
        }

        /**
         * @brief Create a dotted line style
         * @param spacing Space between dots in pixels
         */
        static line_style dotted(float spacing = 4.0f) {
            line_style style;
            style.type = line_style_type::dotted;
            style.spacing = spacing;
            return style;
        }

        bool operator==(const line_style& other) const = default;
    };
}
