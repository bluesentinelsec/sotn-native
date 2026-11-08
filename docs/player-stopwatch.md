# Selected stopwatch

The stopwatch (subweapon 6) has moved from the bounded [subweapon service](laboratory-subweapons.md)
to the shared player runtime, after the [dagger](player-dagger.md), [axe](player-axe.md) and
[holy water](player-holy-water.md). Every subweapon now runs on the shared runtime. Hold Up and tap
either attack button. Defaults, grants and saves are unchanged.

## Use

The shared `func_8010EB5C` path ([Vibhuti](player-vibhuti.md#use)) with `g_SubwpnDefs[6]`:

- **Cost:** twenty hearts (Heart broach applies).
- **Chain:** one watch at a time.
- **Factory:** blueprint 71 (entity 41, slots 32..47, origin 0).
- **Animation:** 0xB8, with the shared casting grunt.
- **Pause gate:** while a stage script sets `g_unkGraphicsStruct.pauseEnemies` (Death, the Chapel
  Maria/confessional events, Hippogryph's Maria), the use is refused before any cost and the
  ordinary hand attack proceeds.

## Watch

`EntityStopWatch` takes three GT4 rows (the dial on page 0x1E, two digits on page 0x1A, all
`DRAW_UNK02` room words) and rolls no attack.

- **Birth.** Refused while paused.
  - Flags: `FLAG_POS_CAMERA_LOCKED | KEEP_ALIVE_OFFCAMERA | HAS_PRIMS | FLAG_POS_PLAYER_LOCKED |
    FLAG_UNK_20000`.
  - It requests `FACTORY(75)` (the circle) and plays ALERT_TINK.
  - It sets `D_800973FC` (the enemy freeze).
- **Hover.** Every update before the exit it closes a twelfth of the X distance and a quarter of
  the Y distance to a point eight pixels behind and sixteen above the player.
  - It turns once it has crossed behind the player (or comes within a pixel).
  - Post-stage shoves move it while it is player-locked.
- **Countdown.**
  - It grows in, tilting through a full turn, rises 26 pixels and settles to 16.
  - It counts five seconds of eighty updates with tick sounds; between ticks it hops with two
    rand calls (higher right after a tick) and swings its tilt.
  - A digit pops in and out each second, its UVs from the digit strip. The tens branch is
    unreachable because the count starts at five.
- **Exit.** At the end, or as soon as `pauseEnemies` is set:
  - It unlocks from the player, re-anchors above the dial and plays SUBWEAPON_TINK in white.
  - Fifteen updates later it requests `FACTORY(4, 14)`, the profile-14 streak burst.
  - It then clears `D_800973FC` and retires.
- **Paused updates.** Every update while paused also clears `D_800973FC`.

`EntityStopWatchExpandingCircle` (entity 42, slots 32..63 from the top) draws sixteen dithered
additive wedges (PAL_FILL_WHITE, priority 0xC2) that grow 0x18 pixels an update for thirty
updates. Both entities keep `FLAG_UNK_20000` across room changes.

## The freeze

The runtime owns `D_800973FC` as `SotnBatRuntime.stopwatch` (never saved). These readers use it:

- enemies (selected stage actors stop, with their original exceptions);
- the Marble Gallery clock room;
- Fairy and Sword familiar flashes.

The Fairy rescue cleanup clears it when it removes a live watch (its entity 0x29 search).

These original effects of an active stopwatch are **not yet implemented**, as before:

- the `BP_120` player tint entity (`func_80123F78`);
- the per-update afterimage reset;
- CD music muting (`MuteCd`);
- the engine's menu/map gate (`func_800F27F4`).

## Validation

- **Public tests.**
  - The pause refusal and cancel, the birth, the exact hover and dial corners, and the rise and
    settle.
  - The countdown: hops (exact rand calls), digit UVs/boxes, the exit, burst and retirement.
  - The circle's UVs, growth and lifetime, and profile-14 streaks.
  - The runtime's freeze flag, circle, shove, room retention, pause and Fairy clear.
  - The clock room holding its statue while the runtime's watch is live.
  - Mutation: two mutants survive, both equivalent: the rise never lands exactly on 26, and the
    tilt is never exactly zero at a hop.
- **Private emulator comparison.**
  - It runs the original `EntityStopWatch` and `EntityStopWatchExpandingCircle` on the actual
    DRA/SLUS with a wandering player and injected pauses.
  - 160 contexts and 38,676 callbacks match: 24 paused births, 22 pause cancels, 3,022 hops and
    89 bursts.
- **Private LLDB probe.** It uses the stopwatch in the Laboratory by pad input, facing both ways.
  - A `pauseEnemies` refusal and a chain refusal.
  - A zombie frozen for all 531 updates of the watch, then walking again.
  - The drawn watch, circle and burst.

These use explicit subweapon/hearts/protection/position fixtures, not acquisition, route or
console evidence.

Adapted source: `src/dra/8A0A4.c` and `7E4BC.c` at the pinned reconstructed revision
`7bc54d8157b6bed77e6bd15e5db9461a734fe1e5`, AGPL-3.0-or-later.
