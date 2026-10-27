# Olrox humanoid body

The internal BO0 checkpoint now runs the placed humanoid body's introduction,
movement, teleport, original action selection and attack commands. Physical
Gladius contact uses the existing native combat/defense service and the private
original HP, damage and experience definition. This is one encounter component;
selected [orbs/portals/bats](olrox-orbs-portals.md),
[fireballs/effects](olrox-fire-effects.md) and
[skull columns](olrox-skull-columns.md) now advance. The second form remains separate work.

The introduction retains its strict distance and exact grounded-contact check,
original visit event and alternate reaction to the preceding furnishing's death.
The original music request starts the private boss intro/loop without changing
the arena's silent initial selection. Action selection retains boundary checks,
facing, distance bands, the private action cycle and its below-75%-HP variant.
Dash, backjump, aerial pass, landing and teleport use original fixed-point motion,
collision sensors, animation cursors and fractional-coordinate behavior. Preserve
source quirks, including state twelve's identical leftward velocity for both
facings and the teleport's screen-space clamps.

Body images retain their original palette, blending, fixed birth coordinates and
64-tick opacity lifetime. The body issues the original orb, portal, fireball and
skull commands using reserved children or bounded effect pools. Reserved slots
carry native parent/child generation tokens instead of PS1 pointers. Unrelated
actors cannot be replaced. Orb readiness waits remain real waits: unsupported
children do not automatically acknowledge or advance the command.

The skull/fireball release checks compare the original packed pose and duration
word. A pose number alone is insufficient: release occurs when its duration
reaches zero. The body updates original frame-dependent physical hitboxes.

The first form's death flag starts its original floor recovery, furnishing-block
signal, optional repositioning, transition animation and transformation-child
request. The body then retires. It does not open the boss doors, award the final
defeat record or produce the final reward. The transformation actor remains
unsupported in this batch; no approximate second phase replaces it.

## Validation and remaining work

Public authored fixtures cover strict introduction/contact boundaries, visit and
music requests, alternate activation, action/HP thresholds, facing and motion,
body-image lifetime, owned factories/readiness, packed-cursor event timing,
furnishing ownership, first-form transition/retirement and invalid state/data.
Private comparisons match 23,800 original-US humanoid instruction snapshots across
68 seeded sequences. They cover the normal body states and retirement, retained
motion/animation/hitbox fields, RNG, audio/event requests, and semantic hashes of
child identities, parameters, facing, readiness and fractional positions. Original
movement, animation, collision-helper, factory and decision instructions execute;
isolated adapters supply initialization, a flat floor, audio backend, visit events
and Alucard-afterimage suppression. Child readiness is an explicit component
fixture. This is not complete encounter or console parity.

Muted native Debug and macOS ASan/UBSan LLDB checks load the placed body, complete
the introduction, move and land ordinary Gladius hits, and observe attack commands.
A separate explicit death-flag fixture exercises the transition boundary after
three real hits; it does not represent naturally defeating the first form.
The actual CMake-built app also runs the introduction and normal hits through
LLDB with normal SDL quit. These checks supply an explicit room arrival and
contact protection, preserve user save slots and keep all private inputs/captures
ignored. They do not grant equipment, stats, relics or ongoing player positions.

The [selected encounter](olrox-encounter.md) now connects transformation, second
form and final defeat/reward. Furnishings and connected progression still need
integration. Alucard's
complete afterimage service and the boss's suppression of those images remain
unintegrated, along with full effects/audio/frame parity. BO0 and NO2 stay
disconnected. Do not call the boss, level, Alucard or game complete.

Current integration: the selected [native encounter](olrox-encounter.md) now
connects both forms, real morph/attacks and captured defeat/reward. Earlier
component checks above retain their narrower evidence scope. Furnishings, full
effects/presentation/parity and connected BO0/NO2 travel remain incomplete.

Provenance: `boss/bo0/2D26C.c:func_us_801AD338` and `EntityOlroxAfterImage` at
sotn-decomp `7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later),
reconstructed from the matching user's US instructions where the pinned source
retains assembly stubs. Private authoring uses spimdisasm 1.42.4 and m2c
`c632946b8cfbe8fce442b522dba92dfa72f1f317`; generated analysis/disassembly is not
shipped or executed by the native app. Public code contains bounded C algorithms;
original timelines, action tables, sensors, initialization, artwork and audio
remain private runtime inputs.
