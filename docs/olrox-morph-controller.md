# Olrox morph controller component

The core now reconstructs the original BO0 transformation controller, its
66-point mesh, 68 owned primitives and ordered draw-environment commands.
It now advances the real articulated body in the selected
[native encounter](olrox-encounter.md). Full presentation/parity and connected
travel remain incomplete.

The controller requests its owned monster child, configures the original initial
127x256 target and following 128x128 target, and retains the output quad's facing,
UVs and screen-space coordinates. Both private mesh topologies share interpolated
fixed-point points; their individual private UVs, original priorities and brightness
crossfade remain separate. Signed sixteen-bit brightness and pulse timers preserve
their original wrap and strict transition boundaries. Completion releases the
original transformation signal and places the retained child at the original
arena floor.

One generation owns the stage's morph points. The reserved-child handoff may
replace the preceding humanoid's owned sibling, while placed/unrelated actors
remain protected. Optional primitive exhaustion retries the original initial
state. Point/index/duration validation prevents malformed private data from
escaping the owned mesh or duration sequence. The existing native offscreen
renderer supplies target commit/restoration independently of this controller.

## Validation and remaining work

Public authored tests cover both facings, target dimensions/offsets, mesh/color
commands, full pulse/brightness sequence, child handoff and final positioning,
optional allocation retry, generation reuse and invalid private tables/state.

Private Debug and macOS sanitizer traces match 2,822 original US instruction
snapshots across twelve sequences, including all 66 points, 68 primitive commands,
draw-environment metadata and controller/child state. Initialization, primitive/
environment allocation, audio and a nonclearing retirement observer are isolated
adapters. The original controller and mesh instructions execute. Its child is
created and positioned but is not advanced in these component comparisons.

These isolated comparisons do not establish gameplay or console-rendering parity.
The [integrated encounter](olrox-encounter.md) now advances the articulated body,
owned parts, attacks and defeat/reward. Pending offscreen writes survive skipped
presentation frames. Selected [BO0/NO2 travel](olrox-connections.md) now runs; full parity remains incomplete;
missing child readiness is never silently acknowledged.

Provenance: BO0 `3053C.c` (`func_us_801B053C`, `func_us_801B088C`,
`func_us_801B0930`) at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
All point, index, duration, definition, artwork and audio arrays remain private
runtime inputs. Authoring disassembly/generated analysis is not shipped.
