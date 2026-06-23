//
// Created by igor on 20/06/2026.
//

#pragma once

#include <cstdint>
#include <simplex/collide/types.hh>

namespace simplex::collide {
    using entity_id_t = uint32_t;

    // Height sentinel stamped on a slot while it sits on the storage free list.
    // A live leaf is height 0 and a live internal node is >= 1, so -1 is unambiguous.
    inline constexpr int16_t FREE_NODE_HEIGHT = -1;

    struct node_ptr {
        using value_t = int32_t;
        static constexpr value_t INVALID_NODE = -1;
        value_t value;

        constexpr node_ptr()
            : value(INVALID_NODE) {
        }

        constexpr explicit node_ptr(int32_t x)
            : value(x) {
        }

        node_ptr& operator =(const node_ptr& x) = default;
        node_ptr(const node_ptr& x) = default;

        constexpr explicit operator bool() const noexcept {
            return value != INVALID_NODE;
        }

        constexpr bool operator==(const node_ptr& x) const noexcept {
            return value == x.value;
        }

        constexpr bool operator!=(const node_ptr& x) const noexcept {
            return !(*this == x);
        }

        constexpr value_t operator*() const noexcept {
            return value;
        }
    };

    struct node {
        aabb box;
        node_ptr parent;
        node_ptr left, right; // child indices; -1 for leaves
        union {
            entity_id_t  entity_id; // when a leaf
            node_ptr next; // when free: free-list link
        };
        int16_t height;
        node ()
            : entity_id(0), height(0) {
        }
    };

    constexpr bool is_leaf(const node& n) {
        return !(n.right && n.left);
    }
}
