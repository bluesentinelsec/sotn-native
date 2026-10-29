# Original player outline component

The core now reconstructs entity31's 24 outline profiles, including the ripple
used by Dark Metamorphosis, ordinary MP refill, cursed attacks and later spells.
This is a controller/packet component. Actual shared primitive/factory allocation,
live spell controllers and rendering remain unwired; no new controls are enabled.

The owner supplies a real normal-body image and current player context. The
component follows the live pose and facing, retains birth-time scales/pivots,
requests one original primitive and reports the real allocation-failure retirement.
It never allocates a substitute pool or signals readiness. The owner must supply
the actual exhaustion result, retain/retire its lease and upload the live artwork
before rendering the copied packet at player priority plus two.

Private profiles supply colors, palette and draw mode. The original eight-update
hold precedes each profile's growth/shrink/fade path. Preserve the distinct birth
scale and later behavior of profiles 15/16/18/19, the cursed profile's slower growth,
clamp-before-final-hold and immediate negative-brightness retirement. Transform,
stone, death, missing body and Hellfire's hidden pose retire the child before a
new packet/reset request. Retained child draw flags remain separate from player
flags; ordinary factory births do not inherit player rotation flags.

Geometry preserves signed-word offsets/pivots, byte UV/dimension wrap, mirrored
edges, scale-before-SDK-rotation and the original SDK rounding. Screen translations
move retained packets without ticking phase, color, scale or timers. Live packets
request the actual player's afterimage reset only after successful geometry.
Malformed private profiles or SDK input preserve state/output atomically. Original
profile tables and artwork stay in the user's private data.

Public authored tests cover all profiles in both facings, actual phase/retirement
progress, packet metadata, captured pivots versus live poses, no-tick translation,
form interruption, allocation exhaustion and malformed-state/data atomicity.
Private muted LLDB comparisons match 576 supplied contexts and 8,336 full original
controller/private-art/SDK packet snapshots, including word-boundary positions,
rotation, interrupted lifetimes and real afterimage-helper execution. Primitive
allocation/destruction and SDK CLZ use explicit adapters. Debug and ASan traces
match; they do not establish actual native shared ownership, rasterization, live
spell, full Alucard, route or console parity.

Next integration must connect actual shared factories, primitive leases, texture
uploads, rendering and player reset ownership before enabling dependent spells.
Existing ordinary starts and version-nine saves remain unchanged.

Provenance: selected `dra/7879C.c:EntityPlayerOutline` and
`4B758.c:func_800EB758`, pinned sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
