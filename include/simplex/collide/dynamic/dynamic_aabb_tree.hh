//
// Created by igor on 20/06/2026.
//

#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>
#include <simplex/collide/dynamic/aabb_tree_node.hh>
#include <simplex/collide/dynamic/aabb_tree_storage.hh>
#include <simplex/collide/overlap.hh>
#include <simplex/collide/sweep.hh>

namespace simplex::collide {
    struct tree {
        aabb_storage m_storage;
        node_ptr m_root;

        tree() = default;

        const node& operator [](node_ptr idx) const {
            return m_storage[idx];
        }

        node& operator [](node_ptr idx) {
            return m_storage[idx];
        }
    };

    namespace detail {
        inline
        aabb combine(const node& n, const aabb& box) {
            return aabb::combine(n.box, box);
        }

        inline
        float area(const node& n) {
            return n.box.area();
        }

        inline
        float get_cost_of_child(const node& child, const aabb& new_box) {
            auto new_left_aabb = combine(child, new_box);

            if (is_leaf(child)) {
                return new_left_aabb.area();
            }
            auto old_area = area(child);
            return (new_left_aabb.area() - old_area);
        }

        inline
        int16_t get_height(const tree& t, node_ptr idx) {
            if (!idx) {
                return -1;
            }
            return t[idx].height;
        }

        inline void refit(const tree& t, node& n) {
            n.height = 1 + std::max(get_height(t, n.left), get_height(t, n.right));
            n.box = aabb::combine(t[n.left].box, t[n.right].box);
        }

        // True when `inner` is fully contained in `outer` (closed bounds).
        inline bool contains(const aabb& outer, const aabb& inner) {
            return outer.min.x() <= inner.min.x() && outer.min.y() <= inner.min.y()
                   && inner.max.x() <= outer.max.x() && inner.max.y() <= outer.max.y();
        }
    }

    inline
    node_ptr find_best_sibling(const tree& t, const aabb& new_box) {
        auto itr = t.m_root;
        while (!is_leaf(t[itr])) {
            // 1. Cost of creating a new parent right here, merging with 'index'
            auto combined_aabb = detail::combine(t[itr], new_box);
            auto cost = combined_aabb.area();
            const auto& n = t[itr];
            //  Inherited cost multiplier used as you descend deeper into the hierarchy
            // (This discourages making deeply nested, overlapping strands)
            auto inheritance_cost = cost - detail::area(n);

            // Compute cost of descending into the Left Child.
            // Both children are dereferenced unconditionally: this relies on the
            // full-binary-tree invariant -- every node has either 0 children (a leaf)
            // or exactly 2, never 1. is_leaf() above only guarantees "at least one
            // child is set", so the unchecked t[n.left]/t[n.right] are safe ONLY while
            // the mutators (insert_leaf, the rotations) preserve that invariant. A
            // one-child node would make t[INVALID_NODE] index out of bounds.
            float cost_left = detail::get_cost_of_child(t[n.left], new_box) + inheritance_cost;
            float cost_right = detail::get_cost_of_child(t[n.right], new_box) + inheritance_cost;

            // If the cost of splitting right here is cheaper than descending further into
            // either child, we BREAK the loop. 'index' is our chosen sibling!
            if ((cost < cost_left) && (cost < cost_right)) {
                break;
            }
            if (cost_left < cost_right) {
                itr = t[itr].left;
            } else {
                itr = t[itr].right;
            }
        }
        return itr;
    }

    inline
    node_ptr rotate_right_up(tree& t, node_ptr aptr) {
        // A is right-heavy. Promote B = A->right; A descends to become B's left child.
        // B's two grandchildren (F, G) are redistributed so the TALLER one stays high
        // under B and the shorter one moves down to A. This single grandchild-aware
        // step resolves both the outer (RR) and inner (RL) imbalance cases.
        auto& a = t[aptr];
        const auto bptr = a.right;
        ENFORCE(bptr && !is_leaf(t[bptr])); // right-heavy ⇒ B is internal
        auto& b = t[bptr];
        const auto fptr = b.left;
        const auto gptr = b.right;

        // --- Link B to A's old parent ---
        b.parent = a.parent;
        if (a.parent) {
            if (auto& p = t[a.parent]; p.left == aptr) {
                p.left = bptr;
            } else {
                p.right = bptr;
            }
        } else {
            t.m_root = bptr; // If A was the root of the whole tree, B is now the root
        }
        // --- A becomes B's left child ---
        b.left = aptr;
        a.parent = bptr;
        // --- Redistribute grandchildren: taller stays under B, shorter moves to A ---
        if (detail::get_height(t, fptr) > detail::get_height(t, gptr)) {
            b.right = fptr;
            a.right = gptr;
        } else {
            b.right = gptr;
            a.right = fptr;
        }
        t[a.right].parent = aptr;
        // --- Recalculate metrics (Bottom-Up order!): A is now lower, update it first ---
        detail::refit(t, a);
        detail::refit(t, b);
        return bptr;
    }

    inline
    node_ptr rotate_left_up(tree& t, node_ptr aptr) {
        // A is left-heavy. Promote B = A->left; A descends to become B's right child.
        // Mirror of rotate_right_up: the taller of B's grandchildren stays high under B,
        // the shorter moves down to A, resolving both the outer (LL) and inner (LR) cases.
        auto& a = t[aptr];
        const auto bptr = a.left;
        ENFORCE(bptr && !is_leaf(t[bptr])); // right-heavy ⇒ B is internal
        auto& b = t[bptr];
        const auto fptr = b.left;
        const auto gptr = b.right;

        // --- Link B to A's old parent ---
        b.parent = a.parent;
        if (a.parent) {
            if (auto& p = t[a.parent]; p.left == aptr) {
                p.left = bptr;
            } else {
                p.right = bptr;
            }
        } else {
            t.m_root = bptr; // If A was the root of the whole tree, B is now the root
        }
        // --- A becomes B's right child ---
        b.right = aptr;
        a.parent = bptr;
        // --- Redistribute grandchildren: taller stays under B, shorter moves to A ---
        if (detail::get_height(t, fptr) > detail::get_height(t, gptr)) {
            b.left = fptr;
            a.left = gptr;
        } else {
            b.left = gptr;
            a.left = fptr;
        }
        t[a.left].parent = aptr;
        // --- Recalculate metrics (Bottom-Up order!): A is now lower, update it first ---
        detail::refit(t, a);
        detail::refit(t, b);
        return bptr;
    }

    inline
    node_ptr balance_tree_at_node(tree& t, node_ptr index) {
        // Refits `index` then fully rebalances the subtree it roots to the strict AVL
        // invariant -- |height(left) - height(right)| <= 1 at every node -- returning
        // the (possibly new) subtree root.
        //
        // PRECONDITION: both child subtrees are already strictly balanced. The insert
        // walk guarantees this by rebalancing bottom-up. `index` itself may be off by
        // MORE than one: a new leaf grafted next to a tall internal sibling produces a
        // fresh parent whose imbalance equals the sibling's height.
        //
        // A single rotation removes only one level of imbalance, so recursion is needed.
        // After promoting the taller child, the demoted node descends carrying one
        // grandchild and may still be unbalanced -- we rebalance it (a STRICTLY SMALLER
        // subtree, which guarantees termination). The grandchild-aware rotation then
        // leaves the promoted node balanced once its demoted child is, so a refit (no
        // further rotation) suffices at this level.
        auto& a = t[index];
        if (is_leaf(a)) {
            return index;
        }
        detail::refit(t, a);
        const auto balance = detail::get_height(t, a.right) - detail::get_height(t, a.left);
        if (balance > 1) {
            auto bptr = rotate_right_up(t, index);
            auto& b = t[bptr];
            balance_tree_at_node(t, b.left); // demoted node descended to b->left
            detail::refit(t, b);
            return bptr;
        }
        if (balance < -1) {
            auto bptr = rotate_left_up(t, index);
            auto& b = t[bptr];
            balance_tree_at_node(t, b.right); // demoted node descended to b->right
            detail::refit(t, b);
            return bptr;
        }
        return index;
    }

    // Grafts an already-allocated leaf node (its `box` set, children null, height 0,
    // not currently in the tree) at the SAH-best sibling and rebalances. Used both by
    // insert_leaf (fresh node) and update_leaf (re-grafting the same node), so the
    // caller's handle survives a move.
    inline
    void insert_existing_leaf(tree& t, node_ptr leaf_ptr) {
        if (!t.m_root) {
            // Empty tree: the leaf becomes the root.
            t.m_root = leaf_ptr;
            t[leaf_ptr].parent = {};
            return;
        }

        // Capture the leaf box by value before the next allocate(), which may reallocate
        // the pool and invalidate any node& we hold. After this point we touch nodes
        // only through t[...] (fresh each time).
        const aabb leaf_box = t[leaf_ptr].box;
        const auto sibling_ptr = find_best_sibling(t, leaf_box);

        const auto old_parent_ptr = t[sibling_ptr].parent;
        const auto merged = aabb::combine(t[sibling_ptr].box, leaf_box);
        const auto sibling_height = t[sibling_ptr].height;

        const auto new_parent_ptr = t.m_storage.allocate(0, merged); // may reallocate

        t[new_parent_ptr].parent = old_parent_ptr;
        t[new_parent_ptr].left = sibling_ptr;
        t[new_parent_ptr].right = leaf_ptr;
        t[new_parent_ptr].height = static_cast <int16_t>(1 + std::max(sibling_height, t[leaf_ptr].height));
        t[sibling_ptr].parent = new_parent_ptr;
        t[leaf_ptr].parent = new_parent_ptr;

        if (!old_parent_ptr) {
            t.m_root = new_parent_ptr;
        } else if (t[old_parent_ptr].left == sibling_ptr) {
            t[old_parent_ptr].left = new_parent_ptr;
        } else {
            t[old_parent_ptr].right = new_parent_ptr;
        }

        for (auto index = new_parent_ptr; index;) {
            index = balance_tree_at_node(t, index);
            index = t[index].parent;
        }
    }

    // Inserts an entity with the given box and returns its stable leaf handle. The tree
    // stores the box verbatim -- any enlargement ("fat AABB" margin for moving objects)
    // is a policy the caller applies before handing the box down.
    inline
    node_ptr insert_leaf(tree& t, entity_id_t eid, const aabb& box) {
        const auto leaf_ptr = t.m_storage.allocate(eid, box);
        insert_existing_leaf(t, leaf_ptr);
        return leaf_ptr;
    }

    namespace detail {
        // INTERNAL helper shared by remove_leaf / update_leaf -- not public API (the
        // intermediate "detached" state only those two know how to finish). Unlinks
        // `leaf` from the tree and rebalances, freeing the now-obsolete parent but NOT
        // the leaf itself, and leaves the leaf in a clean standalone state (parent
        // cleared) so it can be freed or re-grafted. Precondition: `leaf` is a live leaf.
        inline
        void detach_leaf(tree& t, node_ptr leaf) {
            if (leaf == t.m_root) {
                // The leaf is the whole tree.
                t.m_root = {};
                return; // root's parent is already null
            }

            // The parent has exactly two children (full-binary invariant): `leaf` and its
            // sibling. Detaching `leaf` makes the parent obsolete, so the sibling takes the
            // parent's place; the parent is freed.
            const auto parent_ptr = t[leaf].parent;
            const node& parent = t[parent_ptr];
            const auto grandparent_ptr = parent.parent;
            const node_ptr sibling_ptr = (parent.left == leaf) ? parent.right : parent.left;

            if (!grandparent_ptr) {
                // Parent was the root: the sibling becomes the new root (already balanced).
                t.m_root = sibling_ptr;
                t[sibling_ptr].parent = {};
            } else {
                // Splice the sibling into the parent's slot under the grandparent.
                t[sibling_ptr].parent = grandparent_ptr;
                auto& grandparent = t[grandparent_ptr];
                if (grandparent.left == parent_ptr) {
                    grandparent.left = sibling_ptr;
                } else {
                    grandparent.right = sibling_ptr;
                }
            }

            t.m_storage.deallocate(parent_ptr);
            t[leaf].parent = {}; // leaf is now fully detached -- no dangling parent

            // Refit + rebalance from the grandparent up (balance_tree_at_node refits each
            // node itself; the loop is a no-op when the parent was the root).
            for (auto index = grandparent_ptr; index;) {
                index = balance_tree_at_node(t, index);
                index = t[index].parent;
            }
        }
    }

    inline
    void remove_leaf(tree& t, node_ptr leaf) {
        if (!leaf) {
            return;
        }
        // A live leaf is the only node with height 0 (internal >= 1, freed slot ==
        // FREE_NODE_HEIGHT), so this also rejects internal nodes and stale/double-removed
        // handles before they can corrupt the tree.
        ENFORCE(is_leaf(t[leaf]) && t[leaf].height == 0);
        detail::detach_leaf(t, leaf);
        t.m_storage.deallocate(leaf);
    }

    // Updates the leaf's bounds to `box`, keeping the SAME handle valid, and returns
    // whether the tree was restructured.
    //
    // Spatial short-circuit: if `box` is already enclosed by the leaf's stored box,
    // nothing changes and false is returned. This is what makes "fat AABB" callers
    // cheap -- they store an enlarged box and pass the object's tight box here, so small
    // moves stay enclosed -- but the tree itself knows nothing about margins; it just
    // skips work when the new box adds no coverage. Otherwise the leaf is detached,
    // re-grafted in place with `box`, and true is returned.
    inline
    bool update_leaf(tree& t, node_ptr leaf, const aabb& box) {
        ENFORCE(leaf && is_leaf(t[leaf]) && t[leaf].height == 0); // live leaf only (see remove_leaf)
        if (detail::contains(t[leaf].box, box)) {
            return false; // already bounded -> no structural change
        }
        detail::detach_leaf(t, leaf); // keeps the leaf node (handle stays valid)
        t[leaf].box = box;
        insert_existing_leaf(t, leaf); // re-graft the same node
        return true;
    }

    // Type-erased convenience callback for the query() overload below. Prefer passing a
    // lambda to the templated query() on hot paths -- it inlines through the descent,
    // whereas std::function adds an indirect call per hit.
    using query_callback_t = std::function<void(entity_id_t, const aabb& box)>;

    namespace detail {
        // Recursive overlap descent. Returns false as soon as the callback asks to stop,
        // true otherwise (so a parent can short-circuit the second child).
        template <class Fn>
        bool query_helper(const tree& t, node_ptr index, const aabb& box, Fn& on_hit) {
            const node& vertex = t[index];
            if (!intersects(vertex.box, box)) {
                return true; // this subtree can't overlap -> prune, keep searching elsewhere
            }
            if (is_leaf(vertex)) {
                if constexpr (std::is_void_v<std::invoke_result_t<Fn&, entity_id_t, const aabb&>>) {
                    on_hit(vertex.entity_id, vertex.box);
                    return true; // a void callback never stops early
                } else {
                    return on_hit(vertex.entity_id, vertex.box); // bool callback: false -> stop
                }
            }
            // Internal node: both children exist (full-binary invariant). The && short
            // -circuits, so a stop request in the left subtree skips the right one.
            return query_helper(t, vertex.left, box, on_hit)
                && query_helper(t, vertex.right, box, on_hit);
        }
    }

    // Reports every leaf whose STORED box overlaps `box`, calling
    //   on_hit(entity_id_t, const aabb& stored_box)
    // for each. `on_hit` may return void (visit all matches) or bool (return false to
    // stop the whole traversal early -- e.g. first-hit or "does anything overlap?").
    //
    // These are broadphase CANDIDATES, not confirmed overlaps: the reported box is the
    // node's stored box (which, with a fat-AABB margin, can be larger than the object's
    // true bounds) and `intersects` treats touching edges as a hit. Callers narrow-phase
    // the candidates against the real geometry.
    template <class Fn>
    void query(const tree& t, const aabb& box, Fn&& on_hit) {
        if (!t.m_root) {
            return;
        }
        detail::query_helper(t, t.m_root, box, on_hit);
    }

    // Type-erased convenience overload (visits all matches).
    inline
    void query(const tree& t, const aabb& box, const query_callback_t& callback) {
        query<const query_callback_t&>(t, box, callback);
    }

    // Type-erased convenience callback for raycast(). Invoked for each leaf whose stored
    // box the ray crosses, as on_hit(entity_id, const aabb& box, const line_hit& box_hit)
    // where box_hit.entry_param is the fraction along the ray (0 = ray.from, 1 = ray.to)
    // at which it enters the candidate's box.
    using raycast_callback_t = std::function<void(entity_id_t, const aabb& box, const line_hit& box_hit)>;

    namespace detail {
        // Recursive ray descent. `t_max` is the current ray clip (shrinks as a closest-hit
        // callback reports nearer hits). Returns false once the callback asks to stop.
        template <class Fn>
        bool raycast_helper(const tree& t, node_ptr index, const segment& ray, float& t_max, Fn& on_hit) {
            const node& vertex = t[index];
            const auto hit = intersect_param(vertex.box, ray);
            // Prune unless the ray, clipped to [0, t_max], actually crosses this box.
            if (!hit || hit->entry_param > t_max || hit->exit_param < 0.0f) {
                return true;
            }
            if (is_leaf(vertex)) {
                if constexpr (std::is_void_v<
                    std::invoke_result_t<Fn&, entity_id_t, const aabb&, const line_hit&>>) {
                    on_hit(vertex.entity_id, vertex.box, *hit);
                    return true; // void callback: visit every crossed candidate
                } else {
                    // float callback: returns the new max fraction in [0,1] (e.g. the true
                    // hit fraction from narrow-phase) to clip the ray; <= 0 stops.
                    const float new_max = on_hit(vertex.entity_id, vertex.box, *hit);
                    if (new_max <= 0.0f) {
                        return false;
                    }
                    t_max = std::min(t_max, new_max);
                    return true;
                }
            }
            // Descend both children, pruned by the (possibly shrunk) t_max; the && short
            // -circuits an early stop. (A closest-hit walk could descend the nearer child
            // first to prune harder -- omitted for clarity; correctness is order-independent.)
            return raycast_helper(t, vertex.left, ray, t_max, on_hit)
                && raycast_helper(t, vertex.right, ray, t_max, on_hit);
        }
    }

    // Reports every leaf whose STORED box the segment `ray` crosses (ray.from -> ray.to,
    // parameter 0..1), calling on_hit(entity_id, const aabb& box, const line_hit& box_hit).
    // `on_hit` may return void (visit every crossed candidate) or float (closest-hit:
    // return the new max ray fraction in [0,1] to clip farther candidates, <= 0 to stop).
    //
    // As with query(), these are CANDIDATES against stored (possibly fat) boxes -- the
    // caller narrow-phases against the real geometry and, for closest-hit, returns the
    // true hit fraction to drive the pruning.
    template <class Fn>
    void raycast(const tree& t, const segment& ray, Fn&& on_hit) {
        if (!t.m_root) {
            return;
        }
        float t_max = 1.0f;
        detail::raycast_helper(t, t.m_root, ray, t_max, on_hit);
    }

    // Type-erased convenience overload (visits every crossed candidate).
    inline
    void raycast(const tree& t, const segment& ray, const raycast_callback_t& callback) {
        raycast<const raycast_callback_t&>(t, ray, callback);
    }
}
