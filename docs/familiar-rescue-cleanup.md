# Shared player-entity cleanup for Fairy rescue

The shared native runtime now supplies atomic retirement of its actual player
factories8..15, selected children16..63 and linked Wolf body. The projectile service
separately retires its actual projectiles and stopwatch. These are cleanup services
for the upcoming Life Apple binding, not an available rescue producer. Native
Fairy/card lifecycle, full FFF/player17, resource restoration, Mist and other owners,
shared namespace integration and whole-session transactions remain unfinished.
All six [Alucard gates](alucard-status.md) remain open at32/34 dispatched states.

Cleanup validates the complete committed shared runtime and every real row's
identity/generation before releasing anything. Missing, duplicated, stale or
untracked rows fail without changing the runtime or stage pool. Particle sentinel
rows participate. Actual stage target marks require acknowledgement first, and
the native caller must consume pending captures/audio/notifications before cleanup.
Allocating a replacement plan can fail without retiring any real owner.

Retirement clears actual factory/child identities and their consumed callback
views. It preserves the common player eight-row lease, Wolf's environment/capture
globals, initialized AluInit vectors, RNG, shared Blink phases/locks, Holy/teleport
points, generation serials, target cursor, screen-flash counters and attack-group
counts. Original DestroyEntity does not clear those global counts; the next ordinary
entity pass recounts them. No callbacks, sounds, clocks, animation or resource work
run during cleanup. Stage actors, blood/orb actors outside this namespace and
Fairy4..7 leases remain intact. A second cleanup is a valid no-op.

The original Fairy query is hexadecimal entity0x29, decimal41: the stopwatch.
It is separate from decimal29's Hellfire flash. The source searches32..63 and clears
the global stopwatch freeze only when that actual actor exists, before destroying
8..63. The native projectile service applies the equivalent check to its actual
stopwatch record, then retires its projectiles while preserving its collision map,
generation serial, clocks, stage pause gate, queued sounds and character. Its
existing projectile slots still require wider shared namespace/primitive ownership;
this operation does not establish that parity.

Public checks cover all eight factory records, cold/live shared children, actual
Blink and particle-sentinel leases, linked Wolf retirement, common/stage/Fairy row
retention, stale-sentinel and orphan-row rollback, unchanged character/RNG/globals,
idempotence and stopwatch/projectile cleanup with retained sounds/pause/generations.
Native checks exercise actual owned medicine effects and captured physical death
in both builds: two contexts,280 real retired rows and six ordered scene draws.
The checks preserve actual common8/Fairy129/stage rows, reject a stale actual row,
and hold player state, HP/MP, RNG, clock, buffs, death generation/scratch and resident
texture words through cleanup and repeated draws.

All220 authored tests pass in Debug, Release and macOS ASan/UBSan, alongside34
Python checks, repository policy, formatting, static SDL linkage and Release ZIP
checks. Earlier original controller/instruction comparisons retain their stated
scope; these cleanup bindings add native ownership evidence.

Those native contexts use supplied medicine ownership, protection/raw lethal
contact and a direct Fairy-effect source. They are separate from ordinary card
activation, actual Life Apple rescue, FFF/player17, acquisition, uninterrupted
traversal and console parity. Physical death supplies a real captured-image
retention context; it is not a substitute for water death or the remaining recovery
owner. Default grants, savev9 and ordinary starts remain unchanged.

Provenance: selected `servant/tt_002/faerie.c`,
`servant/search_for_entity_in_range.h`, `servant/shared_globals.h`,
`dra/66590.c:DestroyEntity` and native owners derived from `dra/7879C.c`,
`dra/8A0A4.c` and `dra/wolfForm.c`, sotn-decomp
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private assets and comparison traces remain outside published source.
