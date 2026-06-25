//
// Created by igor on 20/06/2026.
//

#pragma once

#include <cstddef>
#include <vector>

#include <simplex/collide/dynamic/aabb_tree_node.hh>
#include <failsafe/enforce.hh>

namespace simplex::collide {
    // A pooled, index-addressed node arena for the dynamic AABB tree.
    //
    // Nodes live in one contiguous vector and are referenced by `node_ptr` (a
    // strongly-typed index), never by raw pointer: the vector can reallocate on
    // growth, so any `node*`/`node&` obtained from operator[] is valid only until
    // the next allocate(). Callers hold `node_ptr` values and re-dereference
    // through operator[] freshly each time.
    //
    // Freed slots are recycled via an intrusive singly-linked free list threaded
    // through the `next` field (which aliases `entity_id`, unused while free); a
    // freed slot is marked with height == FREE_NODE_HEIGHT to catch double frees.
    class aabb_storage {
        public:
            aabb_storage() = default;
            explicit aabb_storage(std::size_t to_reserve);

            // Allocates a node initialised as a leaf (no children, height 0) and
            // returns its index. The index is stable for the node's lifetime; a
            // reference obtained via operator[] is NOT stable across allocate().
            node_ptr allocate(entity_id_t eid, const aabb& box);

            // Returns a freed slot to the free list. A null (default) node_ptr is a no-op.
            void deallocate(node_ptr index);

            // Index -> node access. The reference is valid only until the next
            // allocate() (the backing vector may reallocate).
            node& operator[](node_ptr index) { return m_pool[static_cast <std::size_t>(*index)]; }
            const node& operator[](node_ptr index) const { return m_pool[static_cast <std::size_t>(*index)]; }

            // Total number of slots (live + free). Useful for reserve/validation.
            [[nodiscard]] std::size_t capacity_used() const { return m_pool.size(); }

            // Pre-grow the pool to avoid reallocations during a known batch of inserts.
            void reserve(std::size_t n) { m_pool.reserve(n); }

            // Drops every node and empties the free list -- the arena returns to its
            // just-constructed state. Allocated capacity is retained for reuse. Any
            // node_ptr into the old contents is invalidated.
            void clear() {
                m_pool.clear();
                m_free_head = node_ptr{};
            }

        private:
            static void init(node& n, entity_id_t eid, const aabb& box);

        private:
            std::vector <node> m_pool;
            node_ptr m_free_head;
    };

    inline aabb_storage::aabb_storage(std::size_t to_reserve) {
        m_pool.reserve(to_reserve);
    }

    inline node_ptr aabb_storage::allocate(entity_id_t eid, const aabb& box) {
        node_ptr index;
        if (m_free_head) {
            // Reuse a recycled slot: pop the head of the free list.
            index = m_free_head;
            m_free_head = m_pool[static_cast <std::size_t>(*index)].next;
        } else {
            // Grow the pool. emplace_back may reallocate; that is fine because we
            // resolve the reference (init below) only after the vector settles.
            index = static_cast <node_ptr>(m_pool.size());
            m_pool.emplace_back();
        }
        init(m_pool[static_cast <std::size_t>(*index)], eid, box);
        return index;
    }

    inline void aabb_storage::deallocate(node_ptr index) {
        if (!index) {
            return;
        }
        ENFORCE(*index >= 0 && static_cast<std::size_t>(*index) < m_pool.size());
        node& n = m_pool[static_cast <std::size_t>(*index)];
        ENFORCE(n.height != FREE_NODE_HEIGHT)("Double free of aabb_node"); // guard against double free
        n.height = FREE_NODE_HEIGHT; // mark slot as free
        n.next = m_free_head; // push onto the free list
        m_free_head = index;
    }

    inline void aabb_storage::init(node& n, entity_id_t eid, const aabb& box) {
        n.parent = node_ptr{};
        n.left = node_ptr{};
        n.right = node_ptr{};

        n.height = 0;
        n.entity_id = eid;
        n.box = box;
    }
}
