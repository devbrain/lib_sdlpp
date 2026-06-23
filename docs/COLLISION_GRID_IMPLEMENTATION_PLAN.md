# Static Collision Grid — Design & Implementation Plan

Status: **Draft / proposed.** This is Phase 7 of the collision world
(`docs/COLLISION_WORLD_IMPLEMENTATION_PLAN.md` §12, §17). Target genres: **2D
platformers and shoot-'em-ups (shmups)**, where levels are large and tile-aligned.

The grid is a **uniform spatial index for static geometry** that sits behind the same
query interface as the dynamic AABB tree, so `cast` / `raycast` / `overlap` transparently
consult *both* — the grid for the static world (many tiles), the BVH for the few dynamic
residents.

---

## 1. Goals and non-goals

### Goals
- O(1) static lookups for **tile-aligned** levels (Super Mario / Sonic / Mega Man scale).
- **Implicit storage**: a tilemap *is* the grid — no per-tile body objects, no tree nodes.
- A `static_source` the world queries alongside the BVH, merging results, so callers never
  know there are two structures.
- Support **non-box tiles** (slopes) via the existing `segment` shape — no new narrow-phase.
- Fast ordered ray traversal (DDA) for hitscan / line-of-sight across a level.

### Non-goals
- **No dynamic objects in the grid.** Movers/enemies/bullets stay in the BVH / probe pool;
  the grid is static, built once at level load, never re-bucketed.
- **No new shape types.** Cells yield `aabb` / `segment` / `circle` from the existing
  `shape_t` vocabulary. (Slopes are `segment`s; no triangle/polygon.)
- No grid for the dynamic side; the BVH already handles motion well.

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

## 4. The cell → shape model

The grid is shape-agnostic: traversal/lookup never change with cell contents. A small
**tile-type → shape** table defines what a cell *is*:

```
empty       -> nothing
solid       -> aabb filling the cell:  [c*ts .. (c+1)*ts]
slope_NE/NW/SE/SW -> segment (the diagonal surface), positioned in the cell
half/quarter tiles -> a smaller aabb
```

A query materializes a cell's `shape_t` from `(tile id, cell origin)` on demand and runs the
existing narrow-phase against it. No storage of shapes; no new shape types.

---

## 5. Storage (v1: implicit, bounded)

```
struct grid {
    float       cell_size;       // = tile_size
    vec         origin;          // world position of cell (0,0)'s min corner
    int         w, h;            // grid extent in cells
    std::vector<tile_id> cells;  // w*h, row-major: cells[cy*w + cx]
    // tile_id -> shape table (solid / slope_* / empty / ...), shared/level-owned
};
```

- **Row-major flat array**, `cells[cy*w + cx]` — cache-friendly (a row is contiguous), and
  the only memory beyond the tilemap is the tilemap itself.
- `origin` + `cell_size` place the grid in world space; `world_to_cell` / `cell_to_box` are
  the two conversions everything builds on.
- Out-of-range cells read as `empty` (the level boundary is the caller's concern, or a wall
  ring of solid tiles).

---

## 6. Queries

All three mirror the world's existing query semantics so they can be merged with BVH results.

### 6a. Region / overlap (point & AABB)
Compute the covered cell range `[cx0..cx1] × [cy0..cy1]`, walk those cells, narrow-phase the
probe against each non-empty cell's shape. O(cells covered + their shapes). Feeds `overlap`.

### 6b. Raycast — DDA (Amanatides–Woo), **not plain Bresenham**
A ray crosses a sequence of cells; the **Amanatides–Woo** voxel traversal walks them in
**near-to-far order**, carrying the parametric `t` of each boundary crossing:

```
start at the origin cell
stepX, stepY = sign(dir.x), sign(dir.y)
tMaxX/tMaxY  = param distance to the first vertical / horizontal cell boundary
tDeltaX/tDeltaY = cell / |dir.x| , cell / |dir.y|
loop:
    if tMaxX < tMaxY: cx += stepX; tMaxX += tDeltaX   // crossed a vertical edge
    else:             cy += stepY; tMaxY += tDeltaY   // crossed a horizontal edge
    narrow-phase ray vs the cell's shape; first hit is the nearest -> stop
    stop also when t exceeds the segment length or we leave the grid
```

Because cells are visited in distance order, the **first hit is the closest** and we stop
immediately — no `t_max` pruning bookkeeping (the order gives it for free).

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
- **Small per-frame moves (the common move-and-slide case):** take the **cell rectangle** of
  the swept bound (`enclose(start) ∪ enclose(end)`), narrow-phase the shapes in those cells,
  keep the earliest TOI. No DDA needed.
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

## 8. Integration with the world (the hybrid)

Not "grid instead of BVH" — **both**, behind one interface:

- **Statics → grid** (tiles: O(1), build-once, implicit).
- **Dynamics → BVH** (movers/enemies/items: adapts to motion).
- **`cast` / `raycast` / `overlap` consult both and merge:**
  - `cast` / `raycast`: take the **nearer** of the grid hit and the BVH hit (min TOI).
  - `overlap`: the **union** of grid and BVH hits.
- The caller (move-and-slide, aiming, triggers) issues one query and never knows there are
  two structures.

Proposed seam: a `static_source` the world holds (alongside `m_space_partition`), with
`overlap_region`, `raycast`, `cast` methods over the grid. The world's existing query
methods fan out to both and combine. The grid yields a synthetic identity for cell hits
(e.g. a `collider_id`-like tag encoding the cell, or a reserved "static world" id) so events
can name what was hit.

Open: how a grid cell appears in a `contact` / event — a dedicated `collider_id::type`
(e.g. `TILE`) carrying packed cell coordinates, vs a single sentinel "static" id. (See §11.)

---

## 9. Coordinates, bounds, cell size

- `cell_size = tile_size` exactly → every tile is one cell (the ideal uniform case).
- `origin` aligns cell (0,0) to the level's tile origin; `world_to_cell` floors after
  subtracting `origin`.
- Bounded grid: out-of-range cells are `empty`; a solid border ring (or the world `bounds`
  from the world layer) keeps movers in.
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
3. **How a tile hit appears in events/contacts** — a `collider_id::type::TILE` with packed
   cell coords, or a single "static world" sentinel id? (Affects what the game can do on a
   tile collision — e.g. break a brick.)
4. **Implicit (tilemap) only, or also explicit (bucketed) grid** for free-form statics?
5. **Grid owns vs borrows the tilemap** — zero-copy view over the game's tile array, or its
   own copy?
6. **DDA source** — reuse euler's Bresenham iterator (only if supercover + exposes `t`) or
   implement the Amanatides–Woo loop.

---

## 13. Implementation plan (phased; each phase independently testable)

- **Phase G0 — Grid types & mapping.** `grid` (cell_size, origin, w, h, cells), tile →
  shape table, `world_to_cell` / `cell_to_box` / `cell_shape`. *Tests:* mapping round-trips,
  out-of-range → empty, shape materialization for solid/slope tiles.

- **Phase G1 — Region / overlap query.** Cell-range scan + per-cell narrow-phase. *Tests:*
  brute-force cross-check vs naive cell scan on random tilemaps; filter behavior.

- **Phase G2 — Raycast (DDA).** Amanatides–Woo traversal + per-cell `intersect_param`,
  earliest hit. *Tests:* supercover/no-corner-tunnel, near-to-far ordering, axis-parallel,
  origin-inside, vs brute force.

- **Phase G3 — Swept cast.** Cell-rect sweep (small moves) + earliest TOI. *Tests:*
  anti-tunneling, vs brute force, slope cells.

- **Phase G4 — World integration (hybrid).** `static_source` seam; world `cast`/`raycast`/
  `overlap` fan out to grid + BVH and merge; tile-hit identity in `contact`/events. *Tests:*
  grid-vs-BVH agreement on the same scene; hybrid merge (nearer / union); move-and-slide on a
  tile level; one-way slopes.

- **Phase G5 — (Optional) explicit bucketed grid** for free-form statics, and/or
  height-field slope sampling, if §12 calls for them.

---

## 14. Future / out of scope (v1)
- Multi-resolution / hierarchical grids.
- Dynamic objects in the grid.
- Polygon/triangle slope shapes (slopes stay `segment`s).
- 3D / voxel extension (the DDA generalizes, but out of scope here).
