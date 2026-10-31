# HUD resources and Healing Mail

Owned Healing Mail now queues one HP after128 qualifying walking HUD updates.
The next successful living player update applies it. The original walking status
bit qualifies even against a wall; standing and transformed states reset the
counter. Room changes retain it, menus/map/draws hold it, and full stage/HUD or
Continue initialization resets it. Ordinary starts grant no equipment or relics;
version-nine saves remain unchanged.

The session owns this counter at the actual HUD boundary after player and stage
updates. Its resource work also runs on spell-frozen and release frames, with
the actual retained player status and gameplay clock. MP recovery uses phase0
of the wrapping64-update gameplay clock; equipped Mystic Pendant also enables
phase31. Two Pendants still add only one MP. Status20000000 blocks recovery;
zero HP alone does not. Game-over presentation stops this HUD owner. Saved
playtime is not the resource clock. Fresh character MP binds before the next
player update, so a form or spell gate can use the recovered resource immediately.
Selected Mist and shared ability consumers now receive that actual gameplay clock.

Mail's original kind2/amount1 request overwrites earlier pending blood/orb healing
at the HUD boundary. Later first-pending blood/orb requests preserve it. Applying
kind2 is silent and requests only the actual shared Blink48 owner; kind1 retains
its pickup sound and both Blink48/Blink44 owners. Full HP still requests effects;
stone suppresses flashes. Frozen and dead players retain pending healing without
revival. Failed player updates retain pending state and do not replay committed
one-shot effects. Floating HP-number presentation remains unintegrated for both
healing kinds; this batch introduces no substitute number or effect pool.

Public authored tests cover MP phases, status/cap/repeated-clock/wrap boundaries,
all128 walking counts, reset conditions, invalid-state rollback, request priority,
freeze/death holds, full-HP effects, stone suppression and late player failure.
Private Debug and ASan/UBSan resource traces match byte for byte and match original
HUD instructions across11,648 supplied status/equipment/MP/counter contexts.
Equipment-count adapters supply actual selected counts; these are boundary
comparisons, not complete frame or console replays.

Muted native checks equip actual private Mail/Pendant definitions, verify127
walking updates plus menu hold, the128th deferred request, next-update HP40→41,
one real eight-row Blink48, no blood-only flash/sound, and full-stage counter reset.
Two equipped Pendants recover MP1→2 at actual phase31 despite unrelated saved
playtime, and the next update enters real owned Wolf with that MP. Separate
regressions hold MP through34 real Summon frozen/release updates at phase64 and
recover MP during actual water death without reviving the player. Explicit
equipment/relic, health/protection, clock, pose and queued-arrival fixtures remain
distinct from ordinary acquisition, uninterrupted routes or complete equipment,
Alucard, castle and console parity. No checks write user saves.

Provenance: selected `dra/5F60C.c:DrawHudSubweapon`, `dra/692E8.c` healing/status
and existing shared factory integration, pinned `sotn-decomp`
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5` (AGPL-3.0-or-later).
Private definitions, artwork and comparison traces remain private.
