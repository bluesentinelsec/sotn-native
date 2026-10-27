# Olrox monster head and attack components

Selected components now run the original head's poses, roar timing, laser request
and release signals, aimed fireball bursts and drool. The laser travels to the
floor, sweeps, returns, collapses and releases its damaging effect column, then
retires its owned particles. Fireballs retain their original motion, animated
trails, physical hitboxes and boundary retirement. Drool follows its owned head,
stretches, falls and emits the selected impact sound and shared BO0 dust.

These components now participate in the selected [native encounter](olrox-encounter.md).
Shared damage, real body/morph commands and defeat/reward now advance. Full
presentation/parity and connected travel remain incomplete. Existing ordinary
save formats and normal audio defaults are unchanged.

## Behavior and ownership

The head belongs to its original body generation. Dependent lasers and drool
retire when their head generation expires. A released fireball owns its remaining
lifetime after initialization. Requests wait for real owned attack signals;
unsupported children never acknowledge readiness automatically. The head's roar
checks the original packed pose/duration condition, not the pose alone.

Private tables retain the head/fireball profiles, animation, initialization,
shared dust and SDK inputs. The native laser preserves its triangle as a quad
with a duplicated final vertex. Owned pair metadata replaces the source's use of
primitive packet bytes for mutable particle physics. Sequential XYZ rotation,
column scaling and bounded depth/screen-1024 projection remain separate from the
quarter-wave movement trigonometry.

Laser output uses owned 64x256 targets and ordered environment commands. Shared
[render-target handling](olrox-render-targets.md) now supports original target
replacement and repeated main-surface restores. Existing strict intervals still
reject nesting and unmatched restores. Target replacement commits the previous
owned interval before opening the next; it preserves immutable private artwork.

Optional packet exhaustion retires an attack or omits its capture. Exhausting the
head's laser actor pool fails explicitly rather than following the source's unsafe
fallback that replaces the boss body. These bounded capacity adaptations are not
console parity claims for artificially exhausted pools.

## Validation and limits

Public authored tests cover head roar timing and release waits, expired owners,
projectile hitboxes and retirement, drool impact/dust, invalid screen coordinates,
laser damage/effect phases and complete selected retirement, bounded projection
and ordered target replacement/restoration. Fixtures contain synthetic tables.

Private Debug and macOS sanitizer comparisons match 35,200 original head snapshots
and 6,786 attack/visible-packet snapshots. Head comparisons execute original
controllers, animation, SDK and factories with initialization, allocation, audio
and leading-zero adapters. Attack comparisons execute original controllers and
primitive-helper instructions with an isolated pinned SDK/GTE reference and
allocation/environment/audio/destruction adapters. Flash/shake requests are
isolated because their presentation remains unintegrated. A separate 60,000-pose
SDK/GTE reference comparison checks sequential rotation and scaling.

Head traces seed commands and one release signal without advancing attack
children. Attack traces instead run complete selected lifecycles from static or
moving head fixtures. A private native render check combines two independent
seeded head/laser components, observes 69 overlapping effect frames, verifies
5,254 changed pixels and two retirements, and restores the same deterministic
frame after hiding/showing their packets. This is a render diagnostic; it does
not advance the real monster body or player.

Muted Debug and sanitizer CMake-app diagnostics additionally render two replaceable
begins, drawing, repeated restores and a sample from the final VRAM row, remove
the diagnostic owner before the next simulation step and exit through SDL quit.
Ordinary Entrance startup/quit also remains covered. These checks do not establish
ordinary boss traversal, a completed encounter, whole-frame console parity or
full audiovisual timing. Native flash/shake and full effect parity remain pending.

Provenance: selected BO0 `313A8.c:func_us_801B551C`, `e_laser_attack.c`,
`e_olrox_drool.c`, shared `st/e_misc.h` and `st/prim_helpers.h` at sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later. Selected
bounded matrix/divider math follows the pinned PSY-Z reference documented in
[THIRD_PARTY.md](../THIRD_PARTY.md), MPL-2.0. All original profiles, animations,
artwork, SDK tables and audio remain private runtime inputs. Authoring disassembly,
generated analysis, private fixtures and traces are not shipped.
