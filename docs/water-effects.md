# Ordinary water-effect components

The core reconstructs main splashes, surfacing wakes, side splashes, small drops
and the32-particle drop effect. These components still require actual stage
actors, shared row reservations, playback and rendering before native water
feedback becomes available. The [controller](water-controller.md) emits creation
plans; a plan is not a completed factory or a successful visual allocation.

Main splashes retain their original paired2/4 GT4 packets, adjacent slope packing,
birth-only sound/drop request, unchanged birth position and two motion phases.
Surfacing wakes move and fade on birth, consume one libc-rand value, retain slope
and current behavior and retire at original volume boundaries. Side splashes use
private original vectors with SDK trigonometry, byte age and independent colors.
Undefined original zero-divisor slopes fail explicitly without committing output.

Small drops consume two values from stage Random, retain signed upper parameters,
birth fractions and a single tile. Large drops consume128 values from the separate
libc-rand stream after33 contiguous rows are actually available. Their32 particles
retain packed fractions, delays, visibility and color wrap; the trailing zero-width
sentinel has its own original lifetime. Initial allocation failure retires before
randomness, private reads or sound requests. The components introduce no new pool.

Camera translation shifts actual body and packet coordinates once. Cached world
surface positions stay fixed; packed particle fractions refresh from shifted tile
positions on the next simulation update. Rendering or repeated draws never advance
these effects. The eventual stage owner must preserve actual initialization flags,
generation checks, auxiliary slots and optional drop creation.

Public tests use authored tables and cover birth, fades, lifetimes, motion, separate
random streams, capacity, camera shifts, sentinel ownership and failed-input rollback.
Private comparisons execute the original US instructions over2,464 supplied splash/
wake/side contexts,120,856 updates and349,422 complete GT4 packets, plus512 small/
large-drop contexts,15,632 updates and376,080 packed/sentinel tile snapshots. They
check body state, randomness, sound/drop requests and camera shifts with explicit
primitive, allocation, playback and destruction adapters. These supplied contexts
are not native water traversal, full audiovisual or console parity evidence.

Native placed actors, actual shared primitive/factory/render ownership, player
depth/current feedback and water damage remain separate work. Ordinary starts and
version-nine saves remain unchanged. Alucard and the castle are incomplete.

Provenance: `st/entity_splash_water.h`, `st/water_effects.h` and
`st/entity_water_drop.h`, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Original lookup tables, game inputs and comparison traces remain private.
