# Collision World — Design & Implementation Plan

Status: **Draft / proposed.** Target genres: **2D platformers and shoot-'em-ups (shmups).**

This document specifies the high-level `world` layer that sits on top of the existing
`simplex::collide` narrow-phase queries and the dynamic AABB tree broadphase, and the
phased plan to build it.

---

## 1. Goals and non-goals

### Goals
- A stateful **collision world** that owns colliders, drives the broadphase, runs the
  narrow-phase on candidates, and produces **collision response**.
- Tuned for **platformers** (character move-and-slide vs. static level, slopes, one-way
  platforms, triggers/pickups) and **shmups** (many small fast bullets, layered hitboxes,
  damage/despawn on hit).
- A single ergonomic entry point: `world.run(active_region, dt)` per frame, plus an
  immediate **query** API (`cast` / `overlap` / `raycast`) usable any time.
- **Data-driven response**: adding an interaction is a new material value or a registered
  callback — never a new branch in a hardcoded resolver.

### Non-goals
- **No rigid-body dynamics / constraint solver.** No mass, forces, joints, restitution
  solving, contact islands, or stacking. The only motion model is **kinematic**.
- **No rotated/polygon shapes.** Shapes are `aabb`, `circle`, `segment` (no OBB/polygon);
  these genres don't need rotation, and the existing narrow-phase matrix is built for them.
- No multithreading in v1. No persistence/serialization in v1.

---

## 2. Existing foundation (already built & tested)

Two of the three layers exist and have ground-truth test suites (doctest, ASan/UBSan clean):

**Layer 1 — narrow-phase value types & queries** (`include/simplex/collide/`):
- Shapes: `aabb`, `circle`, `segment`, `vec`; results `line_hit`, `swept_hit`.
- Predicates/queries: `contains`, `intersects`, `overlap` (MTV/penetration),
  `intersect_param` (→ `line_hit`), `swept_intersection` (→ `swept_hit`),
  `closest_point`, `squared_distance`. Pure functions, no state.
- Coverage matrix highlights:
  - `overlap` (MTV: normal + depth): `aabb×aabb`, `circle×circle`, `circle×aabb`.
  - `intersect_param`: `aabb×segment`, `circle×segment`, `segment×segment`.
  - `swept_intersection` (CCD): `aabb×aabb`, `circle×circle`, `circle×aabb`,
    `circle×segment`, `aabb×segment`.

**Layer 2 — broadphase** (`include/simplex/collide/dynamic/`):
- `aabb_tree_node.hh`: `node`, strongly-typed index `node_ptr` (handle), `FREE_NODE_HEIGHT`.
- `aabb_tree_storage.hh`: `aabb_storage` — pooled, index-addressed node arena with a free
  list and double-free guard.
- `dynamic_aabb_tree.hh`: `tree` (a strictly-balanced AVL dynamic AABB tree) with
  `insert_leaf` (returns handle), `remove_leaf`, `update_leaf` (handle-stable, spatial
  containment short-circuit), `query` / `raycast` (template-callback + `std::function`
  overloads, with `bool` early-out), `find_best_sibling`, rotations, `balance_tree_at_node`.
  Pure spatial index — **no entity model, no fat-AABB margin** (deliberately pushed up).
- `sorted_array.hh`: `sorted_set<Storage>` flat sorted set, with `sorted_array<T>`
  (dynamic) and `fixed_sorted_array<T, N>` (inline) aliases.

The world is **Layer 3** and owns everything we deliberately kept out of the tree:
the entity model, the fat-AABB margin, handle lifetimes, layer filtering, response, events.

---

## 3. Architecture overview

```
        game (character controller, bullet system, enemy AI)
                         |  world.run(region, dt) + queries + reactions
   +----------------------------------------------------------------+
   |  Layer 3: collision world  (this document)                     |
   |   - collider/body store (+ generation-tagged handles)          |
   |   - layer filtering, materials, response modes                 |
   |   - movement/CCD pass, contact/behavioral pass, trigger events |
   +----------------------------------------------------------------+
            |  insert/remove/update/query/raycast (node_ptr)
   +----------------------------------------------------------------+
   |  Layer 2: dynamic AABB tree (broadphase)  [done]               |
   +----------------------------------------------------------------+
            |  intersects / overlap / intersect_param / swept_*
   +----------------------------------------------------------------+
   |  Layer 1: narrow-phase queries on aabb/circle/segment  [done]  |
   +----------------------------------------------------------------+
```

### Residents vs. probes (the central pattern)
- **Residents** live in the tree (persistent proxy, updated on move): the **static level**
  and the **enemies** (and usually the **player**). Few, long-lived, need to be *found*.
- **Probes** are not in the tree; each frame they **cast** against it: the **bullets**, and
  the **player's own movement sweep**. Numerous and/or short-lived; they query, they aren't
  queried.

What the tree actually holds: **static level + enemies (+ player) as residents.**
Bullets and movement sweeps are probes. This avoids per-frame tree churn for thousands of
bullets and keeps them in a flat, cache-friendly array.

---

## 4. Core concepts & vocabulary

- **Static**: never moves (tiles, walls, slopes). Built once.
- **Kinematic body**: has position+velocity that *you* control; never moved by forces or by
  other bodies; still collides, blocks/slides, and reports contacts. The player, enemies,
  and moving platforms are kinematic. (There are **no dynamic/rigid bodies** in this design.)
- **Geometric response**: how position/velocity change — block, slide, one-way, bounce,
  friction. Engine-owned, **parameterized by data** (material + response mode).
- **Behavioral response**: gameplay — damage, despawn, pickup, state change. Game-owned;
  the engine only *signals* it via a sparse callback registry.
- **Material**: `{ friction, restitution, response_mode }` on a collider. *Ice is just a low
  friction value* — not a special case.
- **Layer / filter**: category + mask bitsets; a pair collides iff
  `(a.category & b.mask) && (b.category & a.mask)`. Applied before narrow-phase.

---

## 5. Data model (proposed types)

```cpp
namespace simplex::collide {

// Shape a collider can take (matches the narrow-phase matrix; no rotation).
using shape = std::variant<aabb, circle, segment>;

enum class body_kind : uint8_t { static_, kinematic };          // no dynamic/rigid
enum class response_mode : uint8_t { ignore, block, one_way, sensor };

struct material {
    float friction    = 1.0f;
    float restitution = 0.0f;            // 0 = no bounce
    response_mode mode = response_mode::block;
};

struct filter {
    uint16_t category = 0xFFFF;
    uint16_t mask     = 0xFFFF;
};
inline bool should_collide(const filter& a, const filter& b) {
    return (a.category & b.mask) && (b.category & a.mask);
}

// Opaque, generation-tagged handle. Detects use of a freed/recycled slot.
struct collider_id {
    uint32_t index = 0xFFFFFFFFu;
    uint32_t generation = 0;
    [[nodiscard]] bool valid() const { return index != 0xFFFFFFFFu; }
};

// Internal record (pooled, parallel to a generation array).
struct body {
    shape       s;             // local/world shape
    vec         velocity{};    // kinematic motion for this frame (kinematic only)
    material    mat;
    filter      filt;
    body_kind   kind;
    node_ptr    proxy{};       // broadphase handle (resident only)
    uint32_t    generation;
    void*       user = nullptr;// entity id / payload
};

struct contact {
    collider_id a, b;
    vec         normal;        // from a -> b (or surface normal for move-and-slide)
    float       depth = 0.0f;  // penetration (overlap) ...
    float       toi   = 0.0f;  // ... or time-of-impact (swept), per query
};
}
```

`shape` world-space bounds for the broadphase:
- `aabb` → itself; `circle` → center ± radius; `segment` → bbox of endpoints.
A `bounds(shape) -> aabb` helper feeds the tree (fattened by the world's margin).

---

## 6. World API (proposed)

```cpp
class world {
public:
    explicit world(float fat_margin = 0.1f);

    // lifecycle
    collider_id add(const shape&, body_kind, filter, material, void* user = nullptr);
    void        remove(collider_id);
    void        set_shape(collider_id, const shape&);     // resize/teleport
    void        set_velocity(collider_id, vec);           // kinematic intent for next run()
    [[nodiscard]] bool valid(collider_id) const;          // generation check

    // per-frame driver: movement+CCD pass over kinematic bodies, then behavioral pass
    void run(const aabb& active_region, float dt);

    // immediate queries (filtered) -- the probe API (bullets, aiming, AI)
    std::optional<contact> cast(const shape&, vec delta, filter) const; // earliest TOI + normal
    template <class Fn> void overlap(const shape&, filter, Fn&& on_hit) const;
    template <class Fn> void raycast(const segment&, filter, Fn&& on_hit) const;

    // behavioral reactions: sparse registry keyed by category pair
    using reaction = std::function<void(world&, collider_id, collider_id, const contact&)>;
    void on_contact(uint16_t cat_a, uint16_t cat_b, reaction);

    // trigger events (begin/stay/end) for sensor colliders, diffed each run()
    using trigger_cb = std::function<void(collider_id sensor, collider_id other)>;
    void on_begin(trigger_cb);
    void on_end(trigger_cb);
};
```

Notes:
- The **fat margin lives here** (passed to the tree on insert/`update_leaf`); the tree stays
  spatial. Small moves within the fat box don't re-fit the tree.
- `cast` is the move-and-slide / bullet workhorse: broadphase the **swept bound** (union of
  start and end boxes), narrow-phase `swept_intersection` on candidates, return earliest TOI.
- A free helper `move_and_slide(world&, collider_id, vec delta, int max_iter=4)` sits beside
  the core (cast → stop at TOI → project remaining onto surface → repeat). Not baked into
  the engine, so character controllers keep full authority.

---

## 7. The `run` loop

`world.run(active_region, dt)` runs in ordered passes:

1. **Broadphase refresh / cull.** Apply pending `set_shape`/`set_velocity`; `update_leaf`
   moved residents (fat margin). Restrict work to `active_region` (camera bounds + margin).
2. **Movement / solid pass** (kinematic bodies with velocity):
   - For each mover, integrate intended `delta = velocity * dt`.
   - Swept-resolve (CCD) against `block` / `one_way` residents via **move-and-slide**, using
     the *surface's* `material` (friction, restitution, one-way rule).
   - Write back resolved position and velocity. Per-mover-against-the-rest, resolved in
     **TOI order** (see §8 on CCD).
3. **Contact / behavioral pass** (overlaps that aren't geometric blocks):
   - Gather filtered overlapping pairs among residents; for each, invoke the
     `reaction[cat_a][cat_b]` callback if registered (else nothing). Sensors live here.
4. **Trigger events.** Diff this frame's tracked sensor-overlap set against last frame's
   (see §11) → fire `on_begin` / `on_end`.

The **bullet pass is separate** (not part of `run`'s resident loop): the game iterates its
bullet array and calls `world.cast`/`overlap` per bullet (see §10).

---

## 8. Collision response model

The crux of "different response per pair" — solved by **splitting** response and keeping
each half declarative.

### 8a. Geometric response = per-collider material + mode (NOT a pair matrix)
The geometric reaction is a property of the surface being hit, not of the ordered pair:
- `block`   → move-and-slide stops at TOI, slides along the contact normal, friction/restitution from the surface material.
- `one_way` → behaves as `block` only when the mover crosses from the allowed side (e.g. moving downward onto the top); otherwise pass through.
- `sensor`  → no geometric change; generates a contact/event only.
- `ignore`  → filtered out entirely.

Therefore *slope* and *iced platform* are **the same code path**, differing only by
`material.friction`. This collapses what looked like an N×N resolver into **O(N) per-collider
data + one resolver**.

### 8b. Behavioral response = sparse category-pair registry
Gameplay reactions are registered per unordered category pair; almost all cells are empty:

```cpp
world.on_contact(layer::player, layer::enemy_bullet,
    [](world& w, collider_id player, collider_id bullet, const contact&) {
        damage(player);  w.remove(bullet);   // gameplay; engine stays agnostic
    });
```
Dispatch is an O(1) table lookup on `(category_a, category_b)` (normalized to an unordered
key). No virtuals, no per-type code, no giant switch.

### 8c. Move-and-slide / CCD details
- **Per-mover CCD, not simultaneous.** Each mover sweeps against the rest treated as
  stationary for that sub-step; resolve nearest TOI first. (Mutual many-body CCD is out of
  scope and unnecessary for these genres.)
- Dynamic-vs-dynamic (e.g. bullet vs moving enemy) is approximated as swept-vs-current-box.
- Iterate slide a fixed small number of times (`max_iter`); geometry (block) before behavior.
- Ground/wall detection falls out of the contact **normal** (normal ≈ up ⇒ grounded).

---

## 9. Narrow-phase dispatch (the 3×3 matrix)

Broadphase yields candidate pairs by AABB; the world runs the correct shape-vs-shape test by
dispatching on the two `shape` alternatives (`circle`/`aabb`/`segment`) via `std::visit`
(or a manual type-tag switch). Mapping to Layer-1:
- **boolean overlap** (sensors, triggers): `intersects(shape, shape)` — all pairs.
- **MTV** (push-out depth + normal): `overlap(...)` — `aabb×aabb`, `circle×circle`,
  `circle×aabb` only. Segment pairs have no MTV (block-resolve them via swept/`intersect_param`).
- **swept TOI + normal** (`cast`, move-and-slide, bullets): `swept_intersection(...)` —
  `aabb×aabb`, `circle×circle`, `circle×aabb`, `circle×segment`, `aabb×segment`.
- **ray**: `intersect_param(shape, segment)`.

Implication: solid resolution between two `segment`s isn't meaningful (and isn't needed —
the level's solids are `aabb`/`segment` surfaces hit by `aabb`/`circle` movers).

---

## 10. Bullets as probes

- Bullets live in the **game's own flat array** (ideally SoA: positions, velocities, radii),
  not in the world.
- Each frame, per bullet: `world.cast(bullet_shape, bullet_delta, bullet_filter)` (swept →
  anti-tunneling for free). On a hit, apply the behavioral reaction (damage + despawn). No
  geometry.
- Filtering is implicit: player-bullets cast against the enemy filter, enemy-bullets against
  the player. (The player is one object — enemy-bullet-vs-player can be a direct hitbox test,
  no tree needed in that direction.)
- **Caveat:** bullets are *queried-with, not queried-for*. Area effects that act on bullets
  (bomb/bullet-cancel, graze) iterate the bullet array directly — not `world.query`.
- Flip a projectile back to a **resident body** only when it's few/large/slow or must be
  found/destroyed by other systems (e.g. a boss megashot).

---

## 11. Trigger events & the `sorted_array` role

`sorted_array` (specifically `sorted_array<uint64_t>` of packed pair-keys) is the
**persistent trigger-pair set**:
- Pack each sensor-overlap pair as a sorted `(min_id, max_id)` 64-bit key.
- Each `run()`, build the current overlapping-sensor set; **set-difference** against last
  frame's set: keys only in *current* → `on_begin`; only in *previous* → `on_end`; in both →
  stay. Swap the sets.
- The flat sorted representation makes the diff a linear merge and is cache-friendly. (This
  is the consumer that motivated `sorted_array`.)

---

## 12. Static geometry

Decision pending (see §16):
- **BVH residents** (simplest): insert all static colliders into the same tree once. Works
  immediately; no second structure.
- **Uniform grid for tiles** (platformer optimization): a regular tilemap is better served
  by a grid (O(1) cell lookup, trivial swept-DDA — see `docs/DDA_IMPLEMENTATION_PLAN.md`)
  than a BVH. If levels are grid-aligned, support a grid as the *static* collider source and
  keep the BVH for dynamics.

v1 ships with BVH-for-statics; add the grid behind the same query interface if profiling or
tile counts justify it.

---

## 13. Camera / active region

`run` takes an **`aabb` active region**, not a camera object, so collision stays decoupled
from rendering (the caller passes camera bounds + a margin). Caveat: culling collision by the
visible region can drop legitimate near-screen interactions (a bullet just off-screen) — use a
margin and/or an "always-active" flag for the player and homing shots.

---

## 14. Memory & performance

- **Generation-tagged handles** (`collider_id`) over a pooled body store; a stale handle is
  detected (the robustness we deferred at the `node_ptr` level now lives in the world).
- Bodies pooled in a contiguous vector with a free list (mirrors `aabb_storage`).
- Bullets SoA in the game; per-bullet `cast` is a tight loop into a small tree (the reason
  `query`/`raycast`/`cast` are template-callbacks with no `std::function` on the hot path).
- Fat margin amortizes tree re-fits: a mover that stays inside its fat box costs nothing in
  the broadphase that frame.

---

## 15. Testing strategy

Follow the pattern already established for the tree and `sorted_array`:
- **Ground-truth validators**: world invariants (handle/generation consistency, every
  resident has a live proxy whose box bounds its shape, no dangling pairs).
- **Brute-force cross-checks**: `cast`/`overlap`/`raycast` results compared to a linear scan
  over all colliders using the Layer-1 queries directly (the tree must agree with brute force,
  exactly, across random scenes).
- **Scenario tests**: move-and-slide on a slope vs. ice (same path, different friction);
  one-way platform pass/block; bullet tunneling (fast bullet through a thin ship must hit via
  CCD); trigger begin/end edge transitions; layer filtering (no bullet-vs-bullet pairs).
- **Determinism**: same inputs → same results (fixed iteration order; no `Date`/random in
  the engine).
- **ASan + UBSan** on every suite; doctest; standalone-compilable.

---

## 16. Open questions to confirm

1. **Scale / churn**: peak simultaneous kinematic bodies and bullets? (Confirms
   bullets-as-probes and one-tree-with-filter vs. per-team trees.)
2. **Static geometry**: regular tile grid, free-form static AABBs/segments, or both?
   (Grid vs. BVH for statics; whether slopes/segments matter.)
3. **Resolution ownership**: world owns velocity + integration (full `run`-resolves), or
   world provides positions + `cast`/`move_and_slide` helpers and the game integrates?
   (Current plan: world owns the kinematic movement pass; gameplay strictly in callbacks.)
4. **Per-team trees** vs. **one tree + filter** for shmup layer separation.
5. Confirm `sorted_array` is the trigger-pair set (assumed in §11).

---

## 17. Implementation plan (phased; each phase independently testable)

Each phase compiles, has a doctest suite (brute-force cross-check + scenarios), and is
ASan/UBSan clean before the next begins.

- **Phase 0 — Skeleton & types.** `world` shell, `shape` variant, `collider_id` (generation),
  `material`, `filter`, `body` store (pooled + free list), `bounds(shape)`. *Tests:* handle
  validity/recycling, generation invalidation, body store churn (mirror `aabb_storage` tests).

- **Phase 1 — Residency + broadphase glue.** `add`/`remove`/`set_shape` insert/remove/update
  into the tree with the **fat margin**; entity↔proxy mapping. *Tests:* every resident has a
  proxy whose fat box bounds its shape; remove frees the proxy; churn stays bounded.

- **Phase 2 — Filtered queries.** `overlap` / `raycast` / `cast` as filtered wrappers over
  `tree::query`/`raycast` + the 3×3 narrow-phase dispatch. *Tests:* brute-force cross-check
  over random scenes for each query; filter correctness (no excluded pairs reported).

- **Phase 3 — Swept cast + move-and-slide.** `cast` returns earliest TOI + normal; free
  `move_and_slide` helper. *Tests:* slide on flat/slope; anti-tunneling (fast small mover
  through thin wall); corner cases (simultaneous floor+wall).

- **Phase 4 — Geometric response (materials/modes) + movement pass.** `run` movement pass:
  per-mover CCD resolve vs. `block`/`one_way`, friction/restitution from material. *Tests:*
  slope vs. ice (friction), one-way pass/block, grounded detection via normal.

- **Phase 5 — Behavioral reactions.** Category-pair `on_contact` registry; contact pass in
  `run`. *Tests:* player-vs-bullet (damage+despawn) via a registered callback; unregistered
  pairs do nothing; dispatch symmetry (a,b)==(b,a).

- **Phase 6 — Trigger events.** `sorted_array`-backed pair-set diff; `on_begin`/`on_end` for
  sensors. *Tests:* enter/stay/leave transitions fire exactly once each; rapid in/out;
  removal mid-overlap fires `on_end`.

- **Phase 7 — Bullet probe pattern.** Documented helper + example bullet system using `cast`
  (no residency). *Tests:* many-bullets-vs-targets cross-check vs. brute force; tunneling;
  team filtering; perf smoke (N bullets × small tree).

- **Phase 8 — (Optional) static grid.** Uniform-grid static source behind the query
  interface, if §16.2 calls for it. *Tests:* grid vs. BVH agree on the same static scene.

---

## 18. Future / out of scope (v1)
- Rigid/dynamic bodies and a constraint solver.
- Polygon/OBB shapes (rotation).
- Multithreaded broadphase; Morton/cache-aware tree layout (see prior discussion).
- Serialization/replay/determinism hardening beyond fixed iteration order.
