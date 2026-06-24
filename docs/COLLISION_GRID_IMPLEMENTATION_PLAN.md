# Static Collision Grid — Design & Implementation Plan

Status: **In progress (G0 scaffolding).** This is Phase 7 of the collision world
(`docs/COLLISION_WORLD_IMPLEMENTATION_PLAN.md` §12, §17). The coordinate substrate is
implemented in `include/simplex/collide/dynamic/grid.hh` (tested in
`test/simplex/test_grid.cc`): `grid_coord` (with an invalid sentinel), the row-major
`grid_storage<T>`, and `grid<T>` with the physical→cell mapping. The cell→shape
materialization and the queries (region / DDA raycast / swept) are not built yet.
Target genres: **2D platformers and shoot-'em-ups (shmups)**, where levels are large and
tile-aligned.

The grid is a **uniform spatial index for static geometry** that sits behind the same
query interface as the dynamic AABB tree, so `cast` / `raycast` / `overlap` transparently
consult *both* — the grid for the static world (many tiles), the BVH for the few dynamic
residents.

---

## 1. Goals and non-goals

### Goals
- O(1) static lookups for **tile-aligned** levels (Super Mario / Sonic / Mega Man scale).
- A **pure, generic broadphase**: `grid<T>` is a spatial container of an opaque payload `T`
  with cell-enumeration queries — exactly the role the dynamic AABB tree plays, just over a
  lattice instead of a hierarchy. It knows positions and cells; nothing else.
- An **internal property of the world**: the world owns the grid the same way it owns the BVH
  (`m_space_partition`); clients use the world, never the grid directly.
- Fast ordered ray traversal (DDA) for hitscan / line-of-sight across a level.

### Non-goals
- **No semantics in the grid.** The grid has **no knowledge** of `shape_t`, `material_props`,
  `filter_props`, `collider_id`, or `contact` — exactly like the BVH, which only knows boxes
  and entity ids. Cell→shape, materials, filtering, narrow-phase, and contact construction all
  live in the **world** (see §4, §8). The grid only depends on `vec`/`aabb` for lattice math.
- **No dynamic objects in the grid.** Movers/enemies/bullets stay in the BVH / probe pool;
  the grid is static, built once at level load, never re-bucketed.
- **No new shape types** in the engine. The world materializes a cell's geometry as
  `aabb` / `segment` from the existing `shape_t` vocabulary (slopes are `segment`s); the grid
  itself never names a shape.

---

## 2. Why a grid (and when *not*)

A uniform grid maps a world position to a cell with integer arithmetic —
`cx = floor(x / cell), cy = floor(y / cell)` — so spatial lookup is **O(1)**, versus the
BVH's O(log n). The trade is that the grid assumes **uniformity**: it shines when objects
are roughly cell-sized and roughly uniformly distributed, and degrades on wildly varied
sizes ("teapot in a stadium") or sparse/clustered scenes.

| | grid | BVH |
|---|---|---|
| lookup | **O(1)** | O(log n) |
| build | trivial, once | incremental |
| uniform sizes ≈ cell | **ideal** | fine |
| varied sizes / clustered | bad | **ideal** |
| dynamic (moving) | re-bucket every frame | **re-graft cheaply** |

**Tile levels are the grid's perfect case** (every tile is exactly one cell), which is why
statics go in the grid and dynamics in the BVH — each plays to its strength. For SMB-scale
levels the win is mostly **memory and build simplicity**: a flat `W×H` tile array stamped
from the tilemap, versus ~1–2k BVH nodes + balancing. (At SMB scale the per-frame query
cost difference is minor; the grid is chosen for simplicity, not raw speed.)

---

## 3. Core concepts

- **Cell**: a square of fixed `cell_size` (world units). For a tilemap, `cell_size =
  tile_size` so every tile is exactly one cell.
- **O(1) mapping**: `cell(x,y) = (floor(x/cell), floor(y/cell))`. No descent, no compares.
- **Implicit vs explicit grid**:
  - **Implicit** (the tilemap case): there are no stored objects. A cell's tile id, looked
    up in a tile → shape table, *implies* its collision shape, derived on demand from the
    cell coordinate. Storage = the tile array you already have (one id/byte per cell).
  - **Explicit** (free-form statics): each cell holds a bucket of object ids whose AABB
    overlaps it; an object spanning several cells is referenced from each. Dense 2D array
    for a bounded world; spatial **hash** (`map<(cx,cy), bucket>`) for sparse/unbounded.
- **Static / build-once**: the grid is frozen after level load. The grid's weakness
  (re-bucketing moving objects) never fires because statics never move.

v1 targets the **implicit tilemap grid**; the explicit bucketed grid is a later option for
free-form static geometry.

---

## 4. Cell → shape lives in the WORLD, not the grid

The grid is generic over an opaque payload `T` and is **shape-agnostic** — it never names
`shape_t`. Turning a cell into geometry is the **world's** job (the grid is the world's
private broadphase, §8). The world chooses the payload type and the projection:

- `T` is a world type — e.g. a `tile_kind`, or a small struct that also carries an
  **`entity_id_t eid`** (the cell's identity, §8). The grid neither knows nor cares. Anonymous
  geometry (ground/walls) can share a sentinel eid; only interactive tiles (breakable brick,
  `?` block, switch) need a real per-cell eid.
- For each candidate cell a query enumerates, the world maps `(T, cell_box)` → geometry and
  narrow-phases it against the probe, exactly as it already does for a BVH candidate
  (`entity_id` → resident shape). A world-side projection:

  ```
  empty             -> nothing
  solid             -> aabb == cell_box(coord)
  slope_NE/NW/SE/SW -> segment (the diagonal surface) inside the cell
  half/quarter tile -> a smaller aabb
  ```

This keeps the engine's shape vocabulary (and any material/filter a tile carries) entirely on
the world side; the grid stays a pure spatial index. No shapes are stored and no new shape
types are introduced — the world materializes `aabb`/`segment` on demand from `T` + `cell_box`.

---

## 5. Storage (as built, G0)

The cell payload is generic (`grid<T>`); the tile-id-vs-shape decision (§4, §12) is deferred.
The grid is parameterized by **resolution + physical extent**, so the cell size is *derived*
(the texture/heightmap mapping): you say how many cells and what region they cover.

```cpp
namespace detail {
    struct grid_coord {                 // a cell index; INVALID on both axes = "no cell"
        static constexpr uint32_t INVALID = ~0u;
        uint32_t x, y;
        explicit operator bool() const; // valid iff neither axis is INVALID
    };

    template <class T> class grid_storage {     // row-major flat cells, cells[y*w + x]
        grid_storage(uint32_t w, uint32_t h);
        T& operator[](grid_coord);  const T& operator[](grid_coord) const;
        uint32_t get_width() const;  uint32_t get_height() const;
    };
}

template <class T> class grid {
    grid(uint32_t w, uint32_t h, const vec& grid_min, const vec& grid_max);
    // derived: m_cell_dim = (grid_max - grid_min) / (w, h)   -- per-axis, so cells may be
    //          rectangular (square is just the dx/w == dy/h case).
  private:
    detail::grid_coord physical_to_grid(const vec&) const; // floor((p-min)/cell_dim), clamped
};
```

- **Resolution + extent, cell size derived.** Inputs are the cell counts `(w, h)` and the
  world rectangle `[grid_min, grid_max]`; `cell_dim = (grid_max - grid_min) / (w, h)`. There is
  no `cell_size` input. Cells may be **rectangular** (per-axis `cell_dim`); square is the
  `dx/w == dy/h` case. For a tilemap, pass `grid_max = grid_min + (w, h)·tile_size`.
- **Row-major flat array** (`grid_storage<T>`), `cells[y*w + x]` — cache-friendly.
- **`physical_to_grid`** maps a world point to a cell via `floor((p - grid_min) / cell_dim)`,
  **clamped** so the inclusive max edge lands in the last cell (not `w`/`h`), and returns the
  **invalid sentinel** for points outside the bounds (out-of-range → "no cell", never throws) —
  the tolerant behavior the queries need.
- **`cell_box(coord) -> aabb`** is the inverse: the cell's world AABB,
  `[min + coord·cell_dim, + cell_dim]`. Adjacent cells tile contiguously (shared edges), and
  `physical_to_grid(cell_box(c).min) == c` round-trips.

*Remaining (grid side):* the cell-enumeration queries of §6 (region / ordered-ray / swept).
The cell→shape materialization is **not** a grid concern — it lives in the world (§4, §8).

---

## 6. Queries — candidate-cell enumeration (the world narrow-phases)

The grid queries are **candidate enumerators**, mirroring the BVH's contract: they hand back
cells via a callback and do **no narrow-phase** (the grid has no shapes). The boundary is
**fully physical** — inputs are world-space shapes, and the callback receives only **physical
geometry + the opaque payload**: the cell's `cell_box` (a world-space `aabb`) and its `T`. It
does **not** expose `grid_coord` — the world never does grid-coordinate math, and the cell's
identity travels inside `T` (an `eid`, §4/§8). Passing `cell_box` (not the coord) also keeps
`physical_to_grid`/`cell_box` private helpers.

```
template <class Fn> void region (const aabb& box,      Fn&& on_cell) const;   // box-overlap cells
template <class Fn> void raycast(const segment& ray,   Fn&& on_cell) const;   // cells along ray, ordered
template <class Fn> void swept  (const aabb& start, vec delta, Fn&& on_cell) const; // swept-band cells
// on_cell(const T&, const aabb& cell_box [, float t for raycast]) -> void or bool(stop)
// physical in, physical out -- no grid_coord crosses to the world.
```

### 6a. Region / overlap (point & AABB)
Compute the covered cell range `[cx0..cx1] × [cy0..cy1]`, enumerate those cells. The world
narrow-phases the probe against each non-empty cell. O(cells covered). Feeds `overlap`.

### 6b. Raycast — DDA (Amanatides–Woo), **not plain Bresenham**
A ray crosses a sequence of cells; the **Amanatides–Woo** voxel traversal enumerates them in
**near-to-far order**, carrying the parametric `t` of each boundary crossing:

```
start at the origin cell
stepX, stepY = sign(dir.x), sign(dir.y)
tMaxX/tMaxY  = param distance to the first vertical / horizontal cell boundary
tDeltaX/tDeltaY = cell / |dir.x| , cell / |dir.y|
loop:
    if tMaxX < tMaxY: cx += stepX; tMaxX += tDeltaX   // crossed a vertical edge
    else:             cy += stepY; tMaxY += tDeltaY   // crossed a horizontal edge
    on_cell(payload, cell_box, t)                     // world narrow-phases; may signal stop
    stop also when t exceeds the segment length or we leave the grid
```

Because cells are enumerated in distance order, the world's **first confirmed hit is the
closest** — it returns `stop` and the traversal ends, with no `t_max` pruning bookkeeping in
the grid (the order gives it for free).

**Bresenham caveats (do NOT reuse a thin/integer Bresenham blindly):**
- **Corner skipping** → tunneling. Plain Bresenham steps *diagonally* at a cell corner,
  skipping the two corner-adjacent cells. A solid tile there would be missed (a diagonal
  shot leaks through a wall join). Collision needs the **supercover** line (every cell the
  ray touches) — which Amanatides–Woo produces and plain Bresenham does not.
- **No parametric `t`** → Bresenham is integer-only; collision needs `t` at each boundary
  for distance ordering, finite-ray clamping, and contact info. AW carries it (`tMax`).
- *If euler's Bresenham iterator is supercover and exposes the crossing parameter, reuse it;
  otherwise implement the AW loop (it is ~6 lines).*

### 6c. Swept cast (moving AABB/circle)
A swept shape is a **band** of cells, not a thin ray:
- **Small per-frame moves (the common move-and-slide case):** enumerate the **cell rectangle**
  of the swept bound (`enclose(start) ∪ enclose(end)`); the world narrow-phases (swept) each
  cell and keeps the earliest TOI. No DDA needed.
- **Long fast sweeps:** a thickened DDA over the swept band (or fall back to the cell rect of
  the swept bound). Anti-tunneling holds because the swept bound covers the whole path.

---

## 7. Slopes

The grid is the easy part; slope **response** is the hard part, and it is independent of the
grid (same problem in a BVH).

- **Representation:** a slope tile is a **`segment`** (the diagonal surface) in its cell. The
  swept matrix already has `aabb×segment` / `circle×segment` and `intersect_param`, so no new
  narrow-phase is required.
- **Tilemap simplification — one segment per slope cell:** a slope's solid triangle has two
  other faces (vertical back, horizontal base), but in a real tilemap those border
  neighboring **full-block** tiles (full AABBs) that block there anyway. So a slope cell
  usually needs only its diagonal segment. (Extra faces only for a lone floating slope.)
- **Continuity:** a long ramp is a chain of per-cell segments that line up at cell
  boundaries by construction — a mover crossing cells simply hits the next segment. Free.
- **One-way slopes:** `response_mode::ONE_WAY` with `block_normal` = the slope normal works
  unchanged, because the world's one-way rule keys on the **contact normal**
  (`dot(hit_normal, block_normal) > threshold`), which handles a diagonal blocked face.
- **The hard part is response, not the grid:**
  - An **AABB** mover contacts a slope segment at a box corner and must *follow* the surface
    (ground-snap, smooth up/down) rather than catch or jitter — the classic fiddly case.
  - A **circle / capsule** mover rests on a segment at a clean tangent point — much easier.
    Strong argument for a circle-footed player collider (we have `circle`; no capsule yet).
  - Two schools: **generic geometric** (segment + swept/slide via `eval_velocity_response`,
    what the world already does) vs **height-field sampling** (per-column tile heights, the
    Sonic/SMW approach — rock-solid ground-following but a specialized system bolted beside
    the generic collision). v1: generic geometric + circle movers; height-field only if feel
    demands it.

---

## 8. Integration: the world owns everything semantic

The grid is a **private member of the world**, exactly like the BVH (`m_space_partition`).
Clients use the world; the grid is never exposed. The world holds both broadphases and owns
all interpretation:

- **Statics → grid** (tiles: O(1), build-once). **Dynamics → BVH** (movers/enemies/items).
- The world picks the cell payload **`T`** (e.g. `tile_id`), and the **cell → shape /
  material / filter** projection (§4). The grid knows none of these.
- **`cast` / `raycast` / `overlap` fan out to both broadphases and merge.** For each candidate
  — a BVH leaf (`entity_id` → resident shape) *or* a grid cell (`(T, cell_box)` → tile shape)
  — the world runs the **same Layer-1 narrow-phase + filtering**, then combines:
  - `cast` / `raycast`: the **nearer** hit (min TOI/param) across grid and BVH.
  - `overlap`: the **union**.
- The caller (move-and-slide, aiming, triggers) issues one query and never knows there are two
  structures behind it.

**Tile-hit identity is just an `eid`** — uniform with residents and bullets. The cell's `T`
carries an `entity_id_t`; on a confirmed hit the world reads `T.eid` and reports it, exactly
as a resident hit reports its eid. So `cast`/`raycast`/`overlap` against tiles, residents, and
bullets all produce the same event shape — *"X hit `eid`"* — and the game maps `eid → its own
object` and reacts identically. No `collider_id::type::TILE`, no packed cell coords, no
`grid_coord` in the contact. (Internal `collider_id` = slot+generation stays internal to the
pooled stores; the **event-facing identity is the eid**, across all three sources.)

Mutating the grid (e.g. break-brick = clear the cell) is a *separate, spatial* op
(`world.clear_tile(world_pos)`, reconverting internally, or a small `eid → cell` map for the
few destructibles) — the `eid` identifies *what* was hit; it does not pull `grid_coord` into
the query path.

---

## 9. Coordinates, bounds, cell size

- Parameterized by **resolution + extent** (§5): cell counts `(w, h)` over `[grid_min,
  grid_max]`, so `cell_dim = (grid_max - grid_min)/(w, h)` is *derived*. For a tilemap pass
  `grid_max = grid_min + (w, h)·tile_size`, giving `cell_dim == tile_size` (one tile = one
  cell, the ideal uniform case). Cells may be rectangular (`dx/w ≠ dy/h`); square is a special
  case, not a requirement.
- `physical_to_grid` floors `(p - grid_min)/cell_dim`, clamps the inclusive max edge into the
  last cell, and returns the invalid sentinel outside the bounds.
- Bounded grid: out-of-bounds → "no cell" (the world treats that as empty); a solid border
  ring (or the world `bounds` from the world layer) keeps movers in.
- Cell-size tension (general grids): too small → objects span many cells; too large → many
  per cell. For tiles this is a non-issue (one tile = one cell).

---

## 10. Memory & performance

- **Implicit grid memory** = the tilemap (one id/byte per cell); no body objects, no nodes.
- **Build** = O(W·H) to reference the existing tile array (or zero-copy if the grid borrows
  the level's tilemap).
- **Queries**: region O(cells covered), raycast O(cells along ray), swept O(cells in band) —
  all independent of total level size, unlike a linear scan.
- Pairs with the BVH staying tiny (only dynamics), so the world's per-frame iteration over
  *residents* stays small regardless of level size.

---

## 11. Testing strategy

Follow the established pattern (doctest, ASan/UBSan, brute-force cross-checks):
- **Brute-force cross-check**: grid `cast`/`raycast`/`overlap` vs a naive linear scan over
  every non-empty cell's shape using the Layer-1 primitives — must agree exactly on random
  tilemaps.
- **DDA correctness**: supercover (no skipped corner cells → no corner tunneling); near-to-far
  ordering (first hit is nearest); axis-aligned and steep/shallow rays; ray starting inside a
  solid cell; ray parallel to an axis; zero-length ray.
- **Grid vs BVH agreement**: build the *same* static scene as grid cells and as BVH residents;
  every query must return the same hit (the hybrid's core invariant).
- **Slopes**: circle/aabb mover landing on / sliding along a ramp of slope cells; ramp
  continuity across cells; one-way slope pass/block by side.
- **Hybrid merge**: a scene with both a grid wall and a BVH enemy — `cast` returns the nearer;
  `overlap` returns both.

---

## 12. Open questions

1. **Player collider — AABB or circle?** Circle makes slopes dramatically easier (clean
   tangent rest vs box-corner catch). Decides the slope-response difficulty.
2. **Slope response — generic geometric (segment + slide) or height-field sampling?** v1
   leans generic; height-field is a later specialization if feel demands it.
3. **How a tile hit appears in events/contacts — RESOLVED:** as an **`eid`**, uniform with
   residents and bullets. The cell payload `T` carries an `entity_id_t`; the world reports
   `T.eid` on a hit (§8). Anonymous geometry shares a sentinel eid; interactive tiles carry a
   real one. The query boundary stays fully physical (no `grid_coord`, no `TILE` collider type).
   *(Open sub-point: grid mutation/addressing for destructibles — physical `clear_tile(pos)`
   vs an `eid → cell` reverse map — decided when destructibles are built.)*
4. **Implicit (tilemap) only, or also explicit (bucketed) grid** for free-form statics?
5. **Grid owns vs borrows the tilemap** — zero-copy view over the game's tile array, or its
   own copy?
6. **DDA source** — reuse euler's Bresenham iterator (only if supercover + exposes `t`) or
   implement the Amanatides–Woo loop.

---

## 13. Implementation plan (phased; each phase independently testable)

- **Phase G0 — Grid types & mapping. [DONE]** `grid_coord` (invalid sentinel), row-major
  `grid_storage<T>`, `grid<T>` + `physical_to_grid` (clamped, out-of-range → invalid) and the
  inverse `cell_box(coord) -> aabb`, parameterized by resolution + extent (cell size derived;
  rectangular cells supported). *Tests* (`test_grid.cc`): coord validity, storage
  indexing/layout, mapping incl. boundary rounding, max-edge clamp, out-of-bounds → invalid,
  non-origin and rectangular cells; cell_box AABBs, contiguous tiling, and the
  physical_to_grid∘cell_box round-trip. (cell→shape is a world concern, §4 — not part of G0.)

- **Phase G1 — Region cell enumeration.** `region(box, on_cell)` — cell-range scan, callback
  per cell with `(coord, T, cell_box)`. Pure grid (no narrow-phase). *Tests:* the enumerated
  cell set equals the box's cell rectangle on random extents; clipping at the bounds.

- **Phase G2 — Raycast cell enumeration (DDA).** `raycast(ray, on_cell)` — Amanatides–Woo,
  near-to-far, callback `(coord, T, cell_box, t)`, stop on the world's signal. *Tests:*
  supercover/no-corner-skip, near-to-far ordering, axis-parallel, origin-inside, finite-ray
  clamping; cross-check the visited-cell set against a fine point-sample of the ray.

- **Phase G3 — Swept cell enumeration.** `swept(start, delta, on_cell)` — cell-rect of the
  swept bound (small moves). *Tests:* the band covers the whole sweep (anti-tunneling at the
  cell level).

- **Phase G4 — World integration (the semantic layer).** The world holds the grid as a
  private member, defines the cell payload `T` and the cell→shape/material/filter projection
  (§4), and does the per-cell **narrow-phase + filtering** on the grid's enumerated candidates
  (mirroring how it narrow-phases BVH candidates). `cast`/`raycast`/`overlap` fan out to grid +
  BVH and **merge** (nearer / union); tile-hit identity built world-side. *Tests:* grid-vs-BVH
  agreement on the same static scene; hybrid merge (nearer / union); move-and-slide on a tile
  level; one-way slopes.

- **Phase G5 — (Optional) explicit bucketed grid** for free-form statics, and/or
  height-field slope sampling, if §12 calls for them.

---

## 14. Future / out of scope (v1)
- Multi-resolution / hierarchical grids.
- Dynamic objects in the grid.
- Polygon/triangle slope shapes (slopes stay `segment`s).
- 3D / voxel extension (the DDA generalizes, but out of scope here).
