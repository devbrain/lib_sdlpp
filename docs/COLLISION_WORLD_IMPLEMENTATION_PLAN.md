# Collision World — Design & Implementation Plan

Status: **Functionally complete (v1).** Implemented in `include/simplex/collide/dynamic/world.hh`:
Phases 0–6 — skeleton & types, residency + world-owned bullet pool + live-skip iterators,
filtered queries (`overlap`/`cast`/`raycast`/`line_of_sight`), acceptor-aware `cast` +
`move_and_slide` (friction/restitution/one-way), generation-tagged `collider_id` with a null
sentinel, read-back getters, and the full `run()` driver: movement → bullets → triggers, with
region culling, world bounds (`BULLET_EXPIRED`), and a reused event buffer. Tests in
`test/simplex/`: `test_world.cc`, `test_world_queries.cc` (brute-force), `test_world_run.cc`,
`test_world_slide.cc`, `test_enclose.cc` — all ASan/UBSan clean. Remaining: optional static grid
(Phase 7), and a fresh CMake reconfigure to fold the suites into `sdlpp_unittest`. Target genres:
**2D platformers and shoot-'em-ups (shmups).**

This document specifies the high-level `world` layer that sits on top of the existing
`simplex::collide` narrow-phase queries and the dynamic AABB tree broadphase, and the
phased plan to build it. Sections 5, 6 and 17 are kept in sync with the code; where the
implementation has settled a detail differently from the original proposal, the doc
records the **as-built** form and notes what is still a stub.

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
- **No rotated shapes / general polygons.** Shapes are `aabb`, `circle`, `segment`, and a
  solid `triangle` (for slopes/ramps) — no OBB/rotation and no arbitrary N-gon. These genres
  don't need rotation, and the narrow-phase matrix is built for these four.
- No multithreading in v1. No persistence/serialization in v1.

---

## 2. Existing foundation (already built & tested)

Two of the three layers exist and have ground-truth test suites (doctest, ASan/UBSan clean):

**Layer 1 — narrow-phase value types & queries** (`include/simplex/collide/`):
- Shapes: `aabb`, `circle`, `segment`, `vec`; results `line_hit`, `swept_hit`.
- Predicates/queries: `contains`, `intersects`, `overlap` (MTV/penetration),
  `intersect_param` (→ `line_hit`), `swept_intersection` (→ `swept_hit`),
  `closest_point`, `squared_distance`. Pure functions, no state.
- `enclose(shape) -> aabb` (`enclose.hh`): tight bounding box of a `segment` / `aabb` /
  `circle`. constexpr; the world's `fatten` is built on it. Tested in `test_enclose.cc`.
- `translate(shape, vec) -> shape` (`translate.hh`): pure per-shape offset (vec/aabb/circle/
  segment), constexpr `[[nodiscard]]`; the world's move pass uses it to shift a body's shape.
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
- **Behavioral response**: gameplay — damage, despawn, pickup, state change. Game-owned; the
  engine only *reports* it as events returned from `run()`, which the game reacts to (§7/§8b).
- **Material**: `{ friction, restitution, response_mode }` on a collider. *Ice is just a low
  friction value* — not a special case.
- **Layer / filter**: category + mask bitsets; a pair collides iff
  `(a.category & b.mask) && (b.category & a.mask)`. Applied before narrow-phase.

---

## 5. Data model (as built)

The model splits cleanly into a **client-facing** layer (the DTOs you hand to `add`) and an
**internal** layer (the pooled record the world stores). The two are decoupled: the public
vocabulary is ergonomic, the internal layout is optimised independently, and `add`
translates one into the other.

### 5a. Client-facing DTOs (transient input to `add`)

```cpp
using shape_t        = std::variant<segment, aabb, circle>;  // full set: statics / broadphase targets
using moving_shape_t = std::variant<aabb, circle>;           // movers only: NO segment

struct material_props { /* TODO: friction / restitution / response_mode */ };

struct filter_props {
    uint16_t category = 0xFFFF;
    uint16_t mask     = 0xFFFF;
};

struct static_body    { shape_t        shape; material_props material; filter_props filter; };
struct kinematic_body { moving_shape_t shape; material_props material; filter_props filter; vec velocity; };
struct bullet         { moving_shape_t shape; material_props material; filter_props filter; vec velocity; };
```

**Two shape variants give compile-time mover correctness.** A kinematic body / bullet is swept
each frame (it's a *mover*), and a `segment` cannot be a swept mover — that's `raycast`'s job
(a swept line). So movers use `moving_shape_t` (`aabb`/`circle` only): `kinematic_body{ .shape
= segment{…} }` simply **won't compile**. Statics are only ever *targets*, so `static_body`
keeps the full `shape_t` (a slope is a static `segment`). This also lets `cast`'s swept
dispatch cover every `moving_shape_t × shape_t` combination with **no `if constexpr` guard**
(see §9).

The DTOs do **not** inherit (an earlier draft had `kinematic_body : static_body`): the moving
shape is a *narrower* type than the static one, so they're independent structs. They live only
across the `add()` call; `add` flattens them into the internal record below.

Shape allowed per body kind:

| body kind | role in a sweep | shape type |
|---|---|---|
| static | only a target (never moves) | `shape_t` (`segment`/`aabb`/`circle`) |
| kinematic | a mover in `run()` | `moving_shape_t` (`aabb`/`circle`) |
| bullet | a mover (swept each frame) | `moving_shape_t` (`aabb`/`circle`) |

A *moving* slope/platform (rare) must be a thin `aabb`, not a `segment`.

### 5b. Internal records + pooled store (`namespace detail`)

Two record types — residents (in the tree) and bullets (not) — share one pooled store via a
template. The only difference is that bullets carry no `node_ptr proxy`:

```cpp
enum class body_kind { STATIC, KINEMATIC };

struct resident_body {              // flat — NOT a variant of the DTOs (that would re-weld the layers)
    shape_t        shape;           // WIDE: one pool holds both static (maybe segment) + kinematic
    material_props material;
    filter_props   filter;
    vec            velocity{0, 0};  // zero / unused for STATIC
    entity_id_t    eid{};           // the game's id, carried as payload
    body_kind      kind{body_kind::STATIC};
    node_ptr       proxy{};         // broadphase handle (residents only)
    uint32_t       generation{0};   // persistent; bumped on free (never aliased by the free list)
    bool           alive{true};     // O(1) liveness for is_valid + double-free guard
};

struct nonresident_body {           // bullets: MINUS proxy/kind, and TIGHT shape (always a mover)
    moving_shape_t shape; material_props material; filter_props filter;
    vec velocity{0, 0}; entity_id_t eid{};
    uint32_t generation{0}; bool alive{true};
};

template <class Body>               // pooled vector + EXTERNAL free list (std::vector<uint32_t>)
class internal_storage {
    uint32_t allocate();            //   reuse a freed slot (reset, keep generation) or grow
    void     deallocate(uint32_t);  //   ENFORCE(alive) double-free guard; ++generation; push to free list
    bool     is_alive(uint32_t) const;       // bounds-checked liveness
    uint32_t generation(uint32_t) const;     // concept-guarded: 0 if Body has no generation
    Body&    operator[](uint32_t);
    /* live-skip forward iterator: begin()/end() yield only alive slots; iter.index() gives
       the slot so you can deallocate() the current element mid-loop. Marking dead does not
       move storage, and the iterator derefs through the vector, so it survives both
       deallocate-during-iteration and a reallocation from allocate(). */
};
using bodies_storage  = internal_storage<resident_body>;
using bullets_storage = internal_storage<nonresident_body>;
```

Design decisions:
- **`generation` is its own persistent field**, never threaded through the free-list link —
  it must survive the free→alloc cycle (unlike `aabb_storage`'s `next`/`entity_id` union).
- **External free list** (`std::vector<uint32_t>` stack), not an intrusive `next_free`; a
  separate `alive` flag gives O(1) "is this slot free?" for the guard and `is_valid`.
- **One templated store** for both pools; a `has_generation<Body>` concept guards every
  `.generation` access, so the store works with or without the field (lets bullets drop
  generation later if they don't need handles).
- **`material`/`response_mode` are still stubs**; friction/restitution + mode arrive Phase 4.
- **Widen/narrow at the boundary.** The resident pool stores the *wide* `shape_t` (it holds
  static segments too), so `add(kinematic_body)` **widens** `moving_shape_t → shape_t` (always
  valid). The bullet pool stores the *tight* `moving_shape_t` (no conversion). The only narrow
  is `shape_t → moving_shape_t` when `cast` sweeps a *kinematic resident* — its `segment` case
  is unreachable (the typed `add` guarantees it), so the converter's `ENFORCE` is a never-fires
  safety net. `set_shape` is the one runtime guard: a `cid`'s kind isn't known at compile time,
  so setting a kinematic body's shape to a `segment` is rejected with `ENFORCE`, not the type
  system.

### 5c. Handle / identity model (`collider_id`)

```cpp
struct collider_id {
    enum type { BODY, BULLET };          // which pool this handle refers to
    static constexpr uint32_t INVALID = 0xFFFFFFFFu;
    uint32_t value      = INVALID;       // storage slot index (INVALID = null sentinel)
    uint32_t generation = 0;             // must match the slot's current generation
    type     type_id    = BODY;
    [[nodiscard]] bool valid() const { return value != INVALID; }  // cheap null check
};
```

The world owns identity end-to-end: `add` returns a `collider_id{slot, generation, type}`.
For residents the **tree is keyed by the slot index** (`insert_leaf(tree, idx, box)`), so a
broadphase hit resolves directly to `m_bodies_storage[idx]`. The game's own `eid` rides along
as payload in the record. `remove`/`set_*`/`is_valid` branch on `type_id` to hit the right
pool. `is_valid(cid)` is `is_alive(cid.value) && generation(cid.value) == cid.generation` — a
bounds-checked, generation-matched test that detects stale/recycled handles with no UB.

The default-member initializers make a value-initialised `collider_id{}` the **null sentinel**
(`value == INVALID`) instead of aliasing slot 0; `valid()` is the cheap self-check and
`is_valid` also rejects it (INVALID is out of range for any pool). Aggregate init
`collider_id{idx, gen, type}` is unchanged.

### 5d. Still proposed (not yet built)

```cpp
enum class response_mode : uint8_t { ignore, block, one_way, sensor };  // Phase 4

// Returned by cast()/raycast() -- a single query result (Phase 2-3).
struct contact {
    collider_id who;           // what was hit
    vec         normal;        // surface normal / push-out direction
    float       toi   = 0.0f;  // swept time, or penetration depth per query
};

// Emitted by run() into the reused event buffer; the game reacts after run() (§7, §8b).
enum class event_kind { collision, trigger_begin, trigger_end, bullet_hit };  // Phase 4-6
struct world_event {
    event_kind  kind;
    collider_id a, b;          // a = mover/bullet/sensor, b = what it hit / the other
    vec         normal{};
    float       toi{};         // swept time (collision/bullet_hit), else unused
};

inline bool should_collide(const filter_props& a, const filter_props& b) {              // Phase 2 (built)
    return (a.category & b.mask) && (b.category & a.mask);
}
```

Broadphase bounds come from `enclose(shape) -> aabb` (`aabb` → itself; `circle` →
center ± radius; `segment` → bbox of endpoints), fattened by the world's margin (§6/§8c).

---

## 6. World API

### 6a. Implemented (Phase 0 + residency glue)

```cpp
struct world_config { float fatten_margin = 0.1f; };

class world {
public:
    world() = default;
    explicit world(const world_config&);

    // lifecycle -- add overloads take the client DTO + the game's entity id and return a
    // generation-tagged, type-tagged handle (collider_id::BODY for residents, BULLET for the
    // probe pool). Residents go into the broadphase tree; bullets do NOT.
    collider_id add(entity_id_t eid, const static_body&);     // resident (tree)
    collider_id add(entity_id_t eid, const kinematic_body&);  // resident (tree)
    collider_id add(entity_id_t eid, const bullet&);          // probe pool (NOT in tree)
    void        remove(collider_id);                       // no-op on a stale handle
    void        set_shape(collider_id, const shape_t&);    // resize/teleport; ENFORCE valid
    void        set_velocity(collider_id, const vec&);     // ENFORCE valid (+ kinematic for bodies)
    [[nodiscard]] bool is_valid(collider_id) const;        // bounds + generation check
};
```

`add(static_/kinematic_body)` allocates a `resident_body`, fills it from the DTO, computes the
fat box via `fatten`, and inserts a tree proxy keyed by the slot index. `add(bullet)`
allocates a `nonresident_body` in the bullet pool and does **not** touch the tree. `remove`
removes the proxy (residents only) then frees the slot (bumping its generation). `set_shape`
re-fits the proxy through `update_leaf`. The mutator guard policy is deliberate: `remove`
forgives a stale handle (idempotent), the setters `ENFORCE(is_valid)`.

### 6b. Internal helpers (built, not public)

```cpp
    // Resident-vs-resident overlap (intersects double-visit, total over all shape pairs);
    // feeds run()'s contact/trigger passes. Self-excluded; caller dedups pairs.
    template <class Fn> void overlap(uint32_t idx, Fn&& on_hit) const;                // BUILT

    // Swept earliest-TOI query for a *stored* mover (kinematic resident or bullet). Mover is
    // narrowed to moving_shape_t, so the swept dispatch needs no guard (§9). Returns the
    // nearest ACCEPTED contact in [0,1], or nullopt. BODY movers self-exclude. The acceptor
    // runs INSIDE the candidate loop (a post-filter could not recover a farther accepted target
    // behind a rejected near one); a 3-arg convenience overload accepts everything.
    template <class Accept>
    std::optional<contact> cast(uint32_t idx, collider_id::type, vec delta, Accept&&) const;//BUILT
    std::optional<contact> cast(uint32_t idx, collider_id::type, vec delta) const;          //BUILT

    // Per-mover move-and-slide (the movement pass's core; called by run() per kinematic body).
    // Sweeps velocity*dt, stops a `skin` short of each contact, slides the leftover along the
    // surface, damps velocity via the SURFACE material, up to max_slide_iter passes. Mutates the
    // body's shape/velocity + refits its proxy; returns the post-slide velocity, grounded flag,
    // and contacts. `acceptor(const resident_body&)->bool` selects solid surfaces (BLOCK, or
    // ONE_WAY from the blocked side); sensors/ignored return false so they never block.
    template <class Fn> slide_result move_and_slide(uint32_t idx, float dt, Fn&& acceptor);//BUILT
```

Supporting pieces (built):
- `detail::eval_velocity_response(v, n, material)` -- the geometric velocity response
  `v' = (1-friction)*v_t - restitution*v_n` (slide + bounce); surface-owned material, velocity
  only (position slide is pure geometry). `n` unit; `n=={0,0}` and mode-gating are the caller's.
- `detail::translate` (`<simplex/collide/translate.hh>`) -- pure per-shape offset
  (`vec`/`aabb`/`circle`/`segment`); `world::translate(resident_body&, vec)` visits + assigns.
- `slide_result { vec velocity; bool grounded; std::array<contact,4> contacts; int count; }`.
- `world_config` knobs: `skin` (anti-jitter back-off), `max_slide_iter`, `up` (grounded axis),
  `bounds` (optional level extent for BULLET_EXPIRED), plus `GROUND_THRESHOLD ~ 0.707`
  (cos 45 deg) for the grounded test.

### 6c. Public driver (BUILT)

```cpp
    // Per-frame driver: detects + returns events; the GAME reacts after run() returns (§7).
    // The reference points at a buffer the world reuses (no per-frame allocation). Runs the
    // movement (COLLISION), bullet (BULLET_HIT/EXPIRED) and trigger (TRIGGER_BEGIN/END) passes.
    const std::vector<world_event>& run(const aabb& active_region, float dt);          // BUILT

    // Read-back getters (state after run()): resolved shape/position, post-move velocity, eid.
    shape_t     get_shape(collider_id) const;     vec get_velocity(collider_id) const;  // BUILT
    entity_id_t get_eid(collider_id) const;                                             // BUILT
```

Public probe queries (`raycast(segment, filter)`, `line_of_sight(from, to, blockers)`) are also
BUILT. *Still pending (minor):* a thin public `cast(moving_shape_t, delta, filter)` wrapper over
the internal slot-based `cast`.

There is **no** `on_contact` / `on_begin` / `on_end` callback API: behavioral response and
trigger handling are done by the game iterating `run()`'s returned events (§7, §8b, §11).

Notes:
- The **fat margin lives here** (`world_config::fatten_margin`, passed to the tree on
  insert/`update_leaf`); the tree stays spatial. Small moves within the fat box don't re-fit.
- **`fatten(resident_body) -> aabb`** (as built): `enclose` the shape, then —
  - **static** bodies get the *tight* box (they never move, never `update_leaf`, so there is
    no refit to amortise);
  - **kinematic** bodies get a *directional* margin: the leading edge (the one velocity points
    toward) is pushed out by `fatten_margin`; an axis with zero velocity is padded
    symmetrically by `fatten_margin/2` each side (same total span). The fat box always encloses
    the tight box, so the broadphase never under-covers. This is a constant directional margin,
    not velocity-magnitude prediction — fast movers are handled by `run`'s swept query, and a
    `velocity * dt` term can be added in the move pass later if profiling demands it.
- `cast` is the move-and-slide / bullet workhorse: broadphase the **swept bound** (union of
  start and end boxes), narrow-phase `swept_intersection` on candidates, return earliest TOI.
- A free helper `move_and_slide(world&, collider_id, vec delta, int max_iter=4)` sits beside
  the core (cast → stop at TOI → project remaining onto surface → repeat). Not baked into
  the engine, so character controllers keep full authority.

---

## 7. The `run` loop — detect-then-react (events out)

**`run()` returns the events that occurred; the game reacts after it returns.** This is the
core architectural decision: `run()` owns *all* internal iteration (residents, bullets) and
produces an event buffer; the game's reactions — `add`/`remove`/despawn/damage — happen
**between frames**, after `run()` returns and before the next call. Because detection and
reaction are separated in time, there is **no mutation during iteration**:
spawn-during-iteration and death-during-iteration are impossible *by construction*, not
worked around. (This supersedes the earlier callback-registry / game-drives-the-loop model.)

```cpp
const std::vector<world_event>& events = world.run(active_region, dt);  // engine: detect
for (const auto& e : events) react(e);                                  // game: spawn/despawn/damage
```

Return a reference to a buffer the world **reuses** (cleared + refilled each call) so there
is no per-frame allocation.

> **Note — mutations are immediate, not queued.** `set_shape` and `set_velocity` take effect
> the moment they are called (the former refits the proxy synchronously; the latter records
> the velocity). So the broadphase is always current entering `run()`. `set_velocity`'s
> "intent for next `run()`" refers to the **motion** it produces — integrated in the movement
> pass — not to the value being deferred.

Ordered passes inside `run()`:

1. **Cull to the active region.** Restrict this frame's work to `active_region` (camera bounds
   + margin). No pending-change flush is needed (mutations already applied — see note above).
2. **Movement / solid pass** (kinematic bodies with velocity):
   - For each mover, integrate `delta = velocity * dt`.
   - Swept-resolve (CCD) against `block` / `one_way` residents via **move-and-slide**, using
     the *surface's* `material` (friction, restitution, one-way rule).
   - Write back resolved position/velocity and `update_leaf` the mover's proxy (fat margin).
     Per-mover-against-the-rest, in **TOI order** (see §8). Emit a `collision` event per contact.
3. **Bullet pass** (internal — see §10): iterate the world-owned bullet pool (`internal_storage`
   live-skip iterator), integrate + `cast` each ballistic bullet against the tree, and emit a
   `bullet_hit` event per hit. `run()` does **not** despawn bullets — the game does, in reaction
   (so piercing shots simply keep going).
4. **Contact pass** (overlaps that aren't geometric blocks): gather filtered overlapping
   resident pairs (`overlap`, internal) and emit a `collision` event per pair.
5. **Trigger pass.** Diff this frame's sensor-overlap set against last frame's (§11) → emit
   `trigger_begin` / `trigger_end` events.

All five passes append to the one returned event buffer; the game switches on `event_kind`.

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

### 8b. Behavioral response = react to returned events
Gameplay reactions are **not** registered callbacks fired mid-`run()`. The engine reports
*what happened* as events (§7); the game iterates them after `run()` returns and applies
gameplay — the engine stays agnostic and no mutation happens during iteration:

```cpp
for (const auto& e : world.run(region, dt)) {
    if (e.kind == event_kind::bullet_hit && is_player(e.b)) {
        damage(e.b);
        despawn_bullet(e.a);   // GAME decides despawn -> piercing shots just don't
    }
}
```
The game switches on `(event_kind, category_a, category_b)` however it likes (direct switch,
its own dispatch table, ECS system). If a project still wants a registered-callback layer, it
can build one *on top* of the event stream — but the engine's contract is the returned buffer,
not a callback registry. (This replaces the earlier `on_contact` table.)

### 8c. Move-and-slide / CCD details (as built: `world::move_and_slide`)
- **Per-mover CCD, not simultaneous.** Each mover sweeps against the rest treated as
  stationary for that sub-step; resolve nearest TOI first. (Mutual many-body CCD is out of
  scope and unnecessary for these genres.)
- **Loop** (≤ `max_slide_iter`, default 4 — a floor+wall corner needs 2): `cast` the leftover
  → stop a **`skin`** short of the contact (a permanent cushion so the next `cast` does not
  re-hit at toi 0) → slide the leftover `(1 - toi)` of the step with its into-surface component
  removed (`leftover - dot(leftover,n)*n`) → damp **velocity** via `eval_velocity_response`
  (friction/restitution from the SURFACE) → repeat. Bail on an undefined normal (`n == {0,0}`).
- **Position slide is pure geometry; material acts on velocity only** (avoids double-counting;
  the leftover budget uses `toi`, NOT the skin-adjusted advance — the skin is a cushion, not
  part of the motion budget).
- **Solid selection via an `acceptor`** threaded into `cast`: BLOCK always, ONE_WAY only from
  the blocked side (`dot(velocity, block_normal) < 0`); SENSOR/IGNORE return false so they
  never block movement (run() reports them via the overlap/trigger passes instead).
- **Grounded** = any contact with `dot(n, up) > GROUND_THRESHOLD` (~cos 45 deg).
- Dynamic-vs-dynamic (e.g. bullet vs moving enemy) is approximated as swept-vs-current-box.
- The body's shape/velocity and broadphase proxy are updated in place; the `slide_result`
  (post-slide velocity, grounded, contacts) is what run() turns into `collision` events.

---

## 9. Narrow-phase dispatch (the 3×3 matrix)

Broadphase yields candidate pairs by AABB; the world runs the correct shape-vs-shape test by
dispatching on the two `shape` alternatives (`circle`/`aabb`/`segment`) via `std::visit`
(or a manual type-tag switch). Mapping to Layer-1:
- **boolean overlap** (sensors, triggers): `intersects(shape, shape)` — all pairs.
- **MTV** (push-out depth + normal): `overlap(...)` — `aabb×aabb`, `circle×circle`,
  `circle×aabb` only. Segment pairs have no MTV (block-resolve them via swept/`intersect_param`).
- **swept TOI + normal** (`cast`, move-and-slide, bullets): `swept_intersection(...)`. The
  matrix is **mover × target**, and there is **no segment-as-mover** overload (a swept line is
  `raycast`):

  | mover ↓ \ target → | aabb | circle | segment |
  |---|---|---|---|
  | **aabb** | ✓ | ✓ | ✓ |
  | **circle** | ✓ | ✓ | ✓ |
  | segment | ✗ | ✗ | ✗ |

  Because the mover is typed `moving_shape_t` (`aabb`/`circle`), the segment-mover row is
  unconstructable, so `cast`'s `std::visit(mover) × std::visit(target)` is `2 × 3 = 6` combos,
  **all of which have overloads** — no `if constexpr`/`requires` guard is needed. (Contrast the
  partial `overlap`/`intersects` matrices, where the dispatch would otherwise have to guard.)
- **ray**: `intersect_param(shape, segment)`.

Implication: a segment is only ever a swept *target* (a wall/slope hit by an area mover), never
a swept *mover*; solid resolution between two `segment`s isn't meaningful and isn't needed.

---

## 10. Bullets — a world-owned probe pool

Bullets are **probes, not residents**: they query the tree but never live in it (no per-frame
tree churn for thousands of short-lived objects). The pool is **owned by the world** (not the
game's array, as an earlier draft proposed) but kept **out of the broadphase tree**:

- Stored in `detail::internal_storage<nonresident_body>` — the same pooled, free-list,
  mark-dead store as residents, plus a **live-skip iterator**; bullets carry **no `node_ptr`
  proxy** (they're not in the tree).
- Added via `world.add(eid, const bullet&)` (a DTO = `static_body + velocity`, structurally
  like `kinematic_body` but routed to the bullet pool and tagged `collider_id::BULLET`).
- **Swept each frame inside `run()`** (§7 pass 3): the world iterates the live bullets and
  `cast`s each against the tree (swept → anti-tunneling for free). Each hit becomes a
  `bullet_hit` event in the returned buffer.
- **The game decides despawn.** `run()` never removes bullets; it only *reports* hits. The
  game reacts to `bullet_hit` and calls `remove`/despawn — so a **piercing** bullet simply
  doesn't despawn and keeps going. (Decision: game-owned despawn.)
- **Ballistic only for v1.** Bullets fly with their spawn velocity; `run()` owns the
  integrate+cast loop. **Steered bullets (homing/patterns) are deferred** — they'd need the
  game to set per-bullet velocity before `run()` (via the handle) or a steering hook; out of
  scope for now.

Filtering is by `filter_props` (player-bullets vs the enemy mask, etc.). The
enemy-bullet-vs-**player** direction needn't use the tree at all — the player is one object,
so a direct hitbox test suffices; `run()` can special-case it or the game can.

**Caveat — bullets are *queried-with, not queried-for*.** Area effects that act *on* bullets
(bomb/bullet-cancel, graze) can't find them via the tree (bullets aren't in it). Those iterate
the bullet pool directly — exposed for that purpose, or handled inside `run()` as their own
event-producing pass.

Promote a projectile to a **resident body** only when it's few/large/slow or must be
found/destroyed by other systems (e.g. a boss megashot).

---

## 11. Trigger events & the `sorted_array` role

`sorted_array` (specifically `sorted_array<uint64_t>` of packed pair-keys) is the
**persistent trigger-pair set**:
- Pack each sensor-overlap pair as a sorted `(min_id, max_id)` 64-bit key.
- Each `run()`, build the current overlapping-sensor set; **set-difference** against last
  frame's set: keys only in *current* → a `trigger_begin` event; only in *previous* → a
  `trigger_end` event; in both → stay. Swap the sets.
- The flat sorted representation makes the diff a linear merge and is cache-friendly. (This
  is the consumer that motivated `sorted_array`.)

Begin/end are emitted into the same returned event buffer as everything else (§7); there are
no separate `on_begin`/`on_end` callbacks — the game handles `trigger_begin`/`trigger_end`
alongside the other event kinds.

---

## 12. Static geometry

Decision pending (see §16):
- **BVH residents** (simplest): insert all static colliders into the same tree once. Works
  immediately; no second structure.
- **Uniform grid for tiles** (platformer optimization): a regular tilemap is better served
  by a grid (O(1) cell lookup, ordered DDA ray traversal, implicit per-tile storage) than a
  BVH. If levels are grid-aligned, support a grid as the *static* collider source and keep
  the BVH for dynamics. **Full design & phased plan:
  `docs/COLLISION_GRID_IMPLEMENTATION_PLAN.md`** (Phase 7).

v1 ships with BVH-for-statics; add the grid behind the same query interface if profiling or
tile counts justify it. The grid doc covers the cell→shape model (slopes as `segment`s),
Amanatides–Woo DDA (and why plain Bresenham corner-tunnels), and the grid+BVH hybrid merge.

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
- Bodies and bullets pooled in contiguous vectors with free lists (one templated
  `internal_storage`); bullets are out of the tree, swept per frame by `run()`'s internal loop
  (the reason the tree's `query`/`raycast`/`cast` are template-callbacks with no `std::function`
  on the hot path).
- `run()` returns events into a **reused** buffer (cleared/refilled each call) — no per-frame
  allocation, and detection/reaction separation means no mutation during iteration.
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

## 16. Open questions

**Resolved**
- **Identity model**: world owns identity via a generation-tagged, type-tagged
  `collider_id{slot, generation, type}`; the tree is keyed by slot index; the game's `eid` is
  payload on the record. See §5c. (Hardening TODO: a null sentinel on `collider_id`.)
- **`run()` returns events** (detect-then-react), replacing the callback registry. The game
  reacts after `run()` returns, so there is no mutation during iteration. See §7/§8b.
- **Bullets are a world-owned probe pool** (`internal_storage<nonresident_body>`), out of the
  tree, swept inside `run()`. See §10.
- **Despawn-on-hit: the game decides.** `run()` only reports `bullet_hit`; piercing shots just
  don't despawn.
- **Steered bullets: deferred.** v1 bullets are ballistic-from-spawn (§10).
- **Resolution ownership:** the world owns the kinematic movement pass; gameplay lives in the
  game's reaction to returned events (was open question 3).

**Still to confirm**
1. **Scale / churn**: peak simultaneous kinematic bodies and bullets? (one-tree-with-filter
   vs. per-team trees.)
2. **Static geometry**: regular tile grid, free-form static AABBs/segments, or both?
   (Grid vs. BVH for statics; whether slopes/segments matter.)
3. **Per-team trees** vs. **one tree + filter** for shmup layer separation.
4. Confirm `sorted_array` is the trigger-pair set (assumed in §11).

---

## 17. Implementation plan (phased; each phase independently testable)

Each phase compiles, has a doctest suite (brute-force cross-check + scenarios), and is
ASan/UBSan clean before the next begins.

- **Phase 0 — Skeleton & types. [DONE]** `world` shell, `shape_t` variant, `collider_id`
  (generation), `material_props`/`filter_props`, `resident_body` store (pooled + external free
  list, `alive` flag, persistent generation), `enclose(shape)`. *Tests* (`test_world.cc`,
  `test_enclose.cc`): handle validity/recycling, generation invalidation, double-free guard,
  churn; `enclose` for all three shapes incl. constexpr.

- **Phase 1 — Residency + bullet pool + broadphase glue. [MOSTLY DONE]** `add(static_/
  kinematic_body)` insert/remove/update into the tree with the **fat margin** via `fatten`;
  `add(bullet)` into the world-owned `internal_storage<nonresident_body>` (no tree); templated
  pooled store with `has_generation` concept + **live-skip iterator** (deallocate-during-
  iteration + realloc safe); type-tagged `collider_id`; mutator guards. *Done:* lifecycle
  wiring, storage, iterators. *Remaining:* proxy-bounds read-back test; `collider_id` null
  sentinel.

- **Phase 2 — Filtered queries. [DONE]** Internal `overlap` (resident-vs-resident, `intersects`
  double-visit); public `raycast(segment, filter)` (tree raycast + `t_max` pruning) and
  `line_of_sight`. *Tests* (`test_world_queries.cc`): brute-force cross-checks vs linear scans +
  filter/segment/behind-origin scenarios.

- **Phase 3 — Swept `cast` + move-and-slide. [DONE]** `cast` (`moving_shape_t` mover so no
  dispatch guard; acceptor-aware + accept-all overload) brute-force tested (earliest-TOI,
  anti-tunneling, `toi=0` initial-overlap, self-exclusion, filter). `move_and_slide(idx, dt,
  acceptor)` (skin back-off, `(1-toi)` slide, `eval_velocity_response`, grounded, in-place
  writeback) scenario-tested in `test_world_slide.cc` (slope-vs-ice friction, restitution bounce,
  one-way pass/block, floor+wall corners, single-wall slide). `collider_id` null sentinel +
  `valid()` added. *Remaining (minor):* a thin public `cast(moving_shape_t, …)` wrapper.

- **Phase 4 — `run()` movement pass + events. [DONE]** `run(active_region, dt)` returns a reused
  event buffer; movement pass = per-mover CCD vs. solids (BLOCK/ONE_WAY via the acceptor),
  friction/restitution from the surface material, off-region movers culled, emitting `COLLISION`.
  *Tests* (`test_world_run.cc`): floor land/grounded, free fall, zero-velocity, off-region
  dormancy, wall slide, buffer reuse.

- **Phase 5 — Bullet pass. [DONE]** `run()` iterates the world-owned bullet pool, integrates +
  swept-`cast`s each ballistic bullet (region-culled by swept bound), emits `BULLET_HIT`; the
  game despawns (piercing = ignore the event). World `bounds` → `BULLET_EXPIRED`. *Tests:* miss
  full-delta / hit stop-short, cull (off-region skips cast but still flies), fast-entry caught by
  swept bound, expire + game-despawn, unbounded never expires.

- **Phase 6 — Trigger events. [DONE]** The contact pass *is* the trigger pass (response_mode is
  the classifier; no generic per-frame "touch"). Sensor overlaps diffed frame-to-frame →
  `TRIGGER_BEGIN`/`TRIGGER_END` edges. *Tests:* enter/stay/leave once each, removal-mid-overlap
  fires end, solid overlap is not a trigger, sensor filtering. *(Known v1 limit: the slot-index
  pair-key can alias across slot recycling — rare false edge.)*

- **Phase 7 — (Optional) static grid.** Uniform-grid static source behind the query
  interface, if §16.2 calls for it. *Tests:* grid vs. BVH agree on the same static scene.

---

## 18. Future / out of scope (v1)
- Rigid/dynamic bodies and a constraint solver.
- Polygon/OBB shapes (rotation).
- Multithreaded broadphase; Morton/cache-aware tree layout (see prior discussion).
- Serialization/replay/determinism hardening beyond fixed iteration order.

---

## 19. Genre fit & gameplay-collision roadmap

A sanity check of the library against the genres it targets — classic platformers
(Super Mario, Sonic) and shmups (Slordax, Stargunner). The scoping is deliberately a
**kinematic, velocity-driven, swept-CCD arcade layer**, NOT a rigid-body dynamics solver;
that matches all of these genres. So the gaps below are gameplay-collision *idioms*, not
missing dynamics.

### Shmups (Slordax / Stargunner) — covered
Fast bullets vs ships (swept probes, anti-tunnelling), many entities (BVH), off-screen
cull + despawn (`active_region` + `BULLET_EXPIRED`), hit/hurt boxes (`filter` + circle/aabb),
graze (a larger sensor circle on the player). The one real missing capability is
**multi-hit queries** (beams / piercing shots / boss hurtbox scans) — public `world::raycast`
and `cast` return only the NEAREST hit; see "Cross-cutting" below.

### Platformers (Mario / Sonic) — gaps, in priority order
1. **Moving platforms — kinematic-vs-kinematic.** [highest value] *(MP1 carrying DONE; MP2 push /
   MP3 crush pending.)* Modelled actors-and-solids: a **`carrier_body`** is a kinematic "solid"
   (`body_kind::CARRIER`) that moves rigidly on a scripted path and transports its riders. A new
   **carrier pass** runs first in `run()`: each carrier translates by `velocity*dt`, and every actor
   riding on top inherits `(velocity + surface_velocity)*dt` (collision-aware against everything but
   its own carrier; stops at walls). The `surface_velocity` makes a **conveyor** fall out for free
   (carrier `velocity {0,0}`, belt speed in `surface_velocity`); a moving conveyor sets both (rider
   gets the sum). Carriers are excluded from the actor `move_and_slide` pass. *Tested:*
   `test_world_carriers.cc` — horizontal carry, elevator, conveyor (belt doesn't move), moving
   conveyor (sum), non-rider untouched, blocked-at-wall, `set_velocity`/`set_surface_velocity`.
   - **MP2 (pushing) — pending:** a carrier moving *into* an actor displaces it.
   - **MP3 (crushing) — pending:** an actor pinned between a carrier and other solid geometry →
     a `CRUSH` event. (Today a carried rider that can't move simply stops.)
2. **Ground snapping ("stick to the floor" on slopes/stairs).** Walking down a slope/stairs
   should not go airborne each step (and Sonic must hug the ground at speed). Composable
   today (probe down with `raycast`, re-snap after the move using the contact normal +
   `grounded`), but there is no built-in `snap_to_ground` helper — every slope game will
   re-write it.
3. **Step-up / ledge forgiveness.** Distinct from snap-down: a mover walking into a small lip
   (a 1–4 px ledge, the top of a short step) should ride up over it instead of stopping dead,
   when a small upward probe clears. Built from a short up-probe + re-cast, but not provided;
   without it characters catch on tiny tile lips. (Distinct from the tile-seam item below —
   this is intended geometry, not a seam artifact.)
4. **Footing / edge sensors — `ground_support(footprint, max_drop)`.** Detecting that a body
   stands at a ledge (most of its footprint off the edge) drives the **balance/teeter** animation
   (Jazz Jackrabbit, Sonic), **edge-stop** ("don't auto-walk off"), **coyote-time**, and
   **ledge-grab**. This is a STATE the game detects and animates, not a solver behaviour — the
   library's job is the query, not the response (Jazz teeters but does not fall; a true
   tip-over-on-center-of-mass would be dynamics, out of scope). Doable today with a few short
   downward `raycast` probes at footprint offsets (left foot / centre / right foot): supported on
   one side only ⇒ at a ledge. The roadmap helper reports which side(s) are supported and where
   the ledge falls — which is also exactly **Sonic's twin floor sensors (A/B)**. Same small
   raycast-helper family as #2 (ground-snap) and #3 (step-up); no new physics primitive.
5. **Tile-seam snagging → compile the tilemap into a boundary collider.** A flat floor is a
   row of per-cell aabb tiles, so a fast horizontal mover can catch on the shared INTERNAL
   vertical edges between adjacent solids. The fix is not per-tile tuning but a build step:
   compile the solid-tile mask into its BOUNDARY only — merged collinear runs / edge chains /
   boundary faces — so internal edges are never query candidates at all. (Swept + skin only
   masks the symptom.)
6. **Depenetration pass.** No "resolve an existing overlap" step — the world is swept-only.
   The `penetration`/MTV math exists in `collide` (overlap.hh) but is not used by the world,
   so a body shoved into geometry (spawned overlapping, squeezed by a crusher) has no
   relaxation.
7. **Sonic is a sensor-based controller** (floor/wall/ceiling ray sensors + ground angle for
   loops/walls), not an AABB-slide model. The `raycast` primitives can build it, but
   `move_and_slide` is the Mario/Celeste style and will not, by itself, produce loops —
   the Sonic controller is game-level. (Mario-style SOLID ramps, by contrast, are now well
   covered: a `triangle` tile is a free-standing ramp that blocks from every side, not just
   across a diagonal — see grid-plan §15 #4. Only loop/wall-running remains controller-level.)

### Cross-cutting roadmap items (both genres)
- **Multi-hit queries (`raycast_all` / `cast_all`).** Public `raycast`/`cast` return the
  NEAREST hit. Add all-hits variants, ordered by `toi`, with early-out and an optional
  max-hit count. Covers beams, piercing bullets, melee/sword arcs, explosion sweeps, and
  boss multi-hurtbox scans. The grid enumerators already support visit-all callbacks; this is
  the world-level public surface for it (plus the BVH fan-out, no `t_max` clipping).
- **Swept / crossing triggers.** Triggers are an overlap DIFF at frame boundaries, so a fast
  body can cross a thin pickup / checkpoint / tripwire BETWEEN frames without ever overlapping
  it at a sampled endpoint. Add sensors tested along the mover's delta (a swept overlap), or a
  helper built on `cast_all`, so thin/fast crossings still fire.
- **Entity-level collider grouping.** One entity often has several colliders by role — hurtbox,
  hitbox, graze box, weak points, shield zones. The primitives already express these (several
  bodies sharing an eid + `filter`), but events/queries may need game-side dedup by eid or by
  collider role — especially with multi-hit queries (one beam should not report the same enemy
  five times). Worth a small convention (a role tag on the handle, or eid-dedup in the helper).

### Genre-specific niceties (optional)
- **Conveyor / surface velocity** — a tile/material that imparts a tangential velocity to
  bodies resting on it (a `material` extension).
- **Drop-through one-way platforms** — pressing down to fall through a `ONE_WAY` platform;
  mostly game-level (temporarily filter out that platform for that body).

### Correctly NOT needed for these genres
Rigid-body dynamics, stacking/joints, mass/torque, rotation/OBB — sprites rotate visually
while hitboxes stay aabb/circle. Their absence (§18) is deliberate, not a gap.

---

## 20. Game loop / integration

§7 describes the `run` loop abstractly; this is the concrete integration. The rhythm is
**set intent → `run()` → react → read back**: the game owns *control* (gravity, input, AI),
the world owns *collision resolution*. The world is **velocity-in** — it applies no gravity
or input; you hand it a velocity and it resolves where that motion actually lands.

### Fixed timestep
Drive `run()` on a fixed `dt` accumulator and interpolate when rendering. A stable step keeps
the swept CCD and the deterministic slot-order iteration honest (same inputs + same `dt` →
same result).

```cpp
// ---- setup (once) ----
world_config cfg;
cfg.bounds = aabb{{0,0}, {level_w, level_h}};        // also the grid extent
cfg.grid   = world_config::grid_config{ {16,16} };   // tile size
cfg.up     = {0,1};
world w(cfg);

for (auto& t : level.tiles) w.add(t.eid, tile_body{t.shape, t.material, t.filter});
auto player = w.add(P, kinematic_body{ aabb{...}, player_mat, player_filter, {0,0} });
// ... enemies, moving platforms (also kinematic_body) ...

// ---- main loop ----
constexpr float DT = 1.0f/60.0f;
float acc = 0;
while (running) {
    poll_input();
    acc += real_delta_seconds();
    while (acc >= DT) { step(w, DT); acc -= DT; }
    render(w, /*interp=*/ acc / DT);
}
```

### One fixed step
```cpp
void step(world& w, float dt) {
    // 1. INTENT -- the game computes each mover's velocity for this frame (gravity + input/AI).
    for (auto& e : movers) {
        vec v = w.get_velocity(e.handle);
        v += GRAVITY * dt;                  // gravity  (game-level)
        v  = apply_input_and_ai(e, v, dt);  // accel, jump impulse, steering
        w.set_velocity(e.handle, v);
    }
    for (auto& p : platforms) w.set_velocity(p.handle, p.scripted_velocity());

    // 2. RESOLVE -- one detect-then-react pass (move_and_slide per kinematic, sweep bullets,
    //    diff triggers). Returns the REUSED event buffer; copy what you keep, don't hold it.
    const auto& events = w.run(camera_region(), dt);

    // 3. REACT
    for (const auto& ev : events) switch (ev.kind) {
        case event_kind::COLLISION:                 // ev.mover hit ev.target, ev.normal, ev.toi
            if (euler::dot(ev.normal, cfg.up) > 0.7f) ground(ev.mover);
            if (is_hazard(ev.target)) hurt(ev.mover);
            break;
        case event_kind::BULLET_HIT:
            damage(w.get_eid(ev.target));
            if (ev.target.type_id == collider_id::TILE) w.remove(ev.target); // breakable brick
            despawn(ev.mover);
            break;
        case event_kind::BULLET_EXPIRED: despawn(ev.mover); break;
        case event_kind::TRIGGER_BEGIN:  on_enter(ev.mover, ev.target); break;
        case event_kind::TRIGGER_END:    on_exit (ev.mover, ev.target); break;
    }

    // 4. READ BACK -- resolved shape + POST-slide velocity (friction/restitution applied, the
    //    floor has zeroed your fall). Drives animation and is the base for next frame's intent.
    for (auto& e : entities) { e.shape = w.get_shape(e.handle); e.vel = w.get_velocity(e.handle); }

    // 5. CONTROLLER queries (game-level helpers built on raycast / line_of_sight, §19):
    update_grounded_and_coyote(w, player);   // footing / edge sensors -> teeter, edge-stop
    for (auto& en : enemies) en.sees_player = w.line_of_sight(en.pos, player.pos, sight_filter);

    // 6. MOVING-PLATFORM CARRYING (game-level until §19 #1): a rider grounded on a platform
    //    inherits the platform's delta this frame.
    carry_riders(w, platforms, dt);

    // 7. SPAWN / DESPAWN -- fire weapons (add bullet), reap dead (w.remove), level changes.
    flush_spawns(w); reap_dead(w);
}
```

### Rules that matter
- **Set velocities for ALL kinematic bodies before `run()`** (player, enemies, platforms) —
  `run()` moves them in one slot-ordered pass.
- **Velocity-in:** gravity, jump curves, acceleration, AI are the game's job; the library only
  resolves where the requested motion lands.
- **Read `get_velocity` back after `run()`** — it is the *post-slide* velocity (floor zeroed the
  fall, walls removed the into-wall component). Accumulate next frame's gravity onto THAT, not
  the pre-collision guess, or motion goes sticky/jittery.
- **The event buffer is reused** each `run()` — consume or copy it that frame; never stash the
  reference across frames.
- **Game-level today (until the §19 roadmap folds them in):** gravity, moving-platform carrying
  (#1), and footing/coyote/edge-stop/teeter sensors (#4) — all built on `set_velocity` +
  `raycast`/`line_of_sight`. When those land, steps 5–6 shrink.
- **Fixed `dt`** keeps the swept math and deterministic iteration honest; interpolate in render.

Net integration surface: three calls per entity (`set_velocity` → `run` → `get_shape`/
`get_velocity`) plus event handling.
