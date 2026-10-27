# Transformation resource service

The internal transformation resource service reconstructs the original Bat,
Mist and Wolf relic/MP checks. It returns eligibility and the charge owed for one
simulation tick. It never grants a relic, changes player state or deducts MP itself.
Movement, collision, transformation effects, attacks and controls remain separate
work; this service does not make any of those forms playable.

| Form | Required active relic | Strict MP requirement | Active-form charge |
|---|---|---|---|
| Bat | Soul of Bat | More than 1 | 1 every 60 simulation ticks |
| Mist | Form of Mist | More than 10 | 10 every 8 simulation ticks |
| Mist with Power of Mist | Form of Mist and Power of Mist | More than 2 | 2 every 30 simulation ticks |
| Wolf | Soul of Wolf | More than 1 | 1 every 120 simulation ticks |

The original wrapping 32-bit game timer controls the modulo checks. Tick zero is
billable. A check-only call never charges; an active reduction charges only on its
boundary. Eligibility still requires strictly more MP than the cost on an unbilled
tick. A found but disabled primary relic blocks the form. Disabled Power of Mist
uses the ordinary Mist rules, and Power of Mist cannot replace Form of Mist.

Callers must apply `spent` exactly once to ordinary character progress and supply
the original simulation timer rather than presentation frames or a local form
counter. Owned progress stays immutable. Invalid world state, pointers or form
identities report an SDL error and preserve the output. Blocked eligibility is a
successful decision with zero cost. No allocation, private asset read, input
binding, save format or ordinary-start grant is introduced.

## Validation

Public fixtures cover ownership/enabled bits, strict MP boundaries, check-only
behavior, each independent charge boundary, tick zero, 32-bit wrap, immutable
progress and unchanged output for malformed input. All 124 tests pass in Debug,
Release and macOS ASan/UBSan; release packaging passes.

A private instruction comparison matches 6,732 decisions against the user's
original `HandleTransformationMP` and `IsRelicActive` instructions, including all
three forms, disabled/enabled relics, upgraded Mist, threshold/large MP values,
charge/check modes and timer wrap. It executes both original routines without SDK
or service stubs; relic/MP/timer snapshots are supplied. Debug and ASan/UBSan traces
are byte-identical and exit successfully through muted LLDB. This is resource
component evidence, not playable-form, whole-route or console parity evidence.

Provenance: `dra/5D5BC.c:HandleTransformationMP` and `IsRelicActive` at pinned
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
Original DRA bytes and instruction traces remain private and are never shipped.
