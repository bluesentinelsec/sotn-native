# Early castle batch review

The approved PRs #33–39 merged bottom-up into main at
`27bf45ce4d70c26529d4fbd548e689c1fa72367f`. Each had eight passing CI checks.
PRs #40–57 also merged bottom-up, each with eight passing checks, into main at
`e74b2b88a71e93cc5a2c8796e7c67050c3f5fadb`. Main CI subsequently passed.
The user waived manual playtests for the overnight continuation on 2026-10-25;
new batches retain automated and private validation with explicit limitations.

## Implemented scope

- [Opening gear](opening-equipment.md), ordinary Alucard sword/shield behavior,
  [relic/permanent pickup progression](relic-progression.md), and the collected
  [Short sword](short-sword.md). Death removes the actual original equipment.
- [Laboratory Maria](laboratory-maria.md), selected original dialogue/speech,
  saved encounter progress and return of control.
- [Connected Marble Gallery/Outer Wall profiles](early-castle-stages.md), their
  ordinary saves, music and private-data resources.
- [Marble encounters](marble-gallery-enemies.md), [linked enemies](marble-linked-enemies.md)
  and selected [clock mechanisms](marble-clock-room.md).
- Outer Wall [ranged encounters](outer-wall-ranged-enemies.md),
  [Spear Guards](outer-wall-spear-guard.md), [Sword Lords](outer-wall-sword-lord.md),
  [Blue Axe Knights](outer-wall-blue-knight.md), [Skeleton Apes](outer-wall-skeleton-ape.md)
  selected [Armor Lord](outer-wall-armor-lord.md) attacks and death,
  and selected [elevator mechanisms](outer-wall-elevator.md).
- Corrected inside-castle Entrance development arrival, both
  [water-room rocks](entrance-rocks.md) and the [breakable stair section](entrance-stairway.md).

The ordinary development character starts with 70 HP and base stats 6. The
inside-castle start skips the original prologue/forest; it is not an authentic new
game. Explicit stage/room starts remain diagnostics with documented development
initialization. Continuing an ordinary slot restores its supported progress.

## Vertical room crossing correction

An input-driven route check exposed a progression defect at the Laboratory
entrance passage. The native transition only rebased coordinates on the world
grid. The original normal-player room transition instead probes upward below
four pixels and places the player 48 pixels higher in the next room. Downward
crossing uses separate airborne/grounded thresholds and 24/48-pixel arrival
adjustments. Losing that overlap left a normal jump too low to clear the ledge.

The session now uses a bounded neighbor probe and applies the vertical adjustment
only after finding and validating that neighbor. Horizontal crossings retain
priority. Fixed-point fractions and velocity survive the rebase. An early probe
with no neighbor leaves an in-bounds player in the current room. Camera-edge
locks, transformation-specific crossing rules and full transition presentation
still require parity work.

This correction adapts `dra/5087C.c:func_800F0CD8` at the pinned reference commit
under AGPL-3.0-or-later; no room records or tables are embedded. Public synthetic
regressions check exact thresholds, both downward cases, horizontal/corner
priority, malformed dimensions/positions and unchanged failure outputs.

## Validation evidence

Public CI uses synthetic fixtures and no original assets. Private native LLDB
checks use the user's ignored dump and SDL's dummy audio driver; they do not
establish real-device listening or original-console audiovisual/timing parity.

Private component checks cover the implemented actor/mechanism families, both
new stage boundaries in both directions, and an isolated Outer Wall ordinary
save/fresh load. Most use explicit starts and protected or granted fixtures;
read each feature document for its scope. They are not an uninterrupted route.

A separate ordinary-input check starts inside Castle Entrance, runs Death's
speech/theft, breaks the water barriers, climbs the towers, collects Cube of Zoe
and the Short sword, and reaches Laboratory's first save room. It uses no room
jumps or resource grants. A private replay matches all 9,622 recorded ticks of
stage, room, fixed-point position and HP under macOS ASan/UBSan. The ordinary save
and fresh native Continue retain exact supported progress, including time and
four owned RNG streams, in an isolated test slot. The sanitizer sequence also
leaves that save room, takes ordinary enemy hits through physical death/game over,
returns to the native Continue shell and restores the same ordinary slot again.
This checks internal repeatability; it does not compare against the console game.
Continuous traversal
through the complete boss/Marble/Wall progression route remains separate
acceptance work; successful isolated checks do not establish it.

All 54 public test groups pass in Debug and macOS ASan/UBSan, and local release
packaging passes. The release archive contains the app, README and licenses,
with no private runtime files; SDL3 remains statically linked.

## Remaining progression work

Doppleganger and its boss-stage transfer remain unintegrated. Other Wall enemies
and hidden mechanisms, additional return-Entrance encounters, the late clock
ritual, transformation abilities, further castle stages, original prologue/title
initialization, and full effects/audio/timing parity remain incomplete. Neither
Marble Gallery nor Outer Wall is a completed authentic area. A fifty-stage
resource catalog does not change that status.

The next focused integration target is the Outer Wall boss/progression boundary
and its missing prerequisite encounters, followed by extending the ordinary
route/replay evidence across that encounter. Original-console comparisons remain
necessary before claiming parity.
