# Original water-volume query

The core now reconstructs the ordinary US stage water-volume query and its three
separate depth values. It supplies no placed actor, native player feedback,
current, splash factory, swimming or water-damage owner yet. Existing live form
controls and ordinary starts remain unchanged.

The query borrows bounded eight-word volume records and the original eight-entry
slope-divisor table from private stage data. It preserves signed-word intermediate
wrapping, truncating division, strict right/bottom edges, inclusive surface/left
edges and the original first-match order. A zero depth can still have an encoded
surface contact. Crouch/form priority selects the original foot sampling offset,
body height and distinct feet/waist/head depths; these values cannot be replaced
by one boolean tile-contact flag.

Original no-contact execution writes some uninitialized scratch into unused cached
fields. The native query initializes unused row/distances to zero and exposes an
explicit contact result. Those fields confer no selection or effect readiness.
Invalid selectors, truncated records and invalid reached slope indices fail with
unchanged output. Missing native ownership must never be replaced with a guessed
volume, water depth, successful splash allocation or default relic grant.

Public tests use authored tables for forms, slopes, edges, first-match priority,
word wrapping and atomic failure. A private comparison runs the original full US
Entrance water actor for13,824 supplied original/copied-volume, slope, form, edge
and scroll contexts, including5,868 contacts. The query and all three depth globals
match; the original auxiliary allocator returns failure through an explicit adapter.
Unwritten no-contact scratch is excluded. This is distinct from actual native
feedback integration. Debug and ASan/UBSan traces match byte for byte.
This component is a foundation for connecting
Alucard's normal/form water behavior, not evidence of playable swimming, complete
Alucard, whole-route or console parity.

Provenance: selected `st/water_effects.h:func_801C4144` and the depth-selection part
of `EntityAlucardWaterEffect`, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later). Original volume,
slope and presentation tables remain private runtime inputs.
