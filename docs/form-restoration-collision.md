# Form restoration collision checks

Internal collision services now reconstruct the original Mist-only barrier escape
and shared floor-edge ejection used when restoring Alucard. They return decisions
without moving the player, changing the map or completing a transformation.
The selected [Mist controller](player-mist.md) now applies their decisions with
real sensor/effect restoration. Bat and Wolf remain unavailable; these pure checks
do not independently complete any transformation.

The barrier check probes seven pixels to either side of the player. A barrier on
one side pushes away at the original three-quarter-pixel speed; barriers on both
sides use the current facing direction. No barrier leaves the owner's velocity
untouched. The caller applies the reported velocity only when `blocked` is true.

Floor-edge checks retain the different left/right slope masks, original second
floor sensor offset and probe positions. A successful ejection reports its whole-
pixel shift using raw tile distances, preserving the caller’s position fraction.
`displaced` can be true when that shift is zero: callers must retain the original
successful-contact behavior instead of testing the shift alone.

Mist reset ignores a Mist-only first probe. The source’s second-probe check clears
the discarded first collider’s effect field; it leaves the second solid result
intact. Native code preserves that behavior. It does not silently clear the second
probe to make a blocked exit pass. Bounded position validation and checked collision
lookups leave outputs unchanged on malformed input. These checks cannot replace
the real transformation-effect readiness wait.

## Validation

Public authored fixtures cover both barrier sides/facings, free movement,
left/right floor ejection, successful zero displacement, first/second Mist probe
behavior, bounds, missing definitions and unchanged failure output. They contain
no private game tables. All 126 test groups pass in Debug, Release and macOS
ASan/UBSan; release packaging passes.

A private comparison matches 61,440 original helper decisions and exact probe
sequences across all 256 collision codes, multiple positions and sensor offsets,
both facings and ordinary/Mist reset modes. Instrumented production source is
checked against the actual CMake functions first. Original helper and facing-speed
instructions execute; the collision adapter supplies query responses from the
native decoder. This is a controller comparison with supplied collision responses,
not an independent PS1 collision decoder, playable transformation or console/route
parity claim. Debug and sanitizer decision/query traces are byte-identical and
exit successfully through muted LLDB. Original instructions and diagnostic
traces remain private.

Provenance: `dra/71830.c:func_80111D24` and `func_80111DE8`, sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
